/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_
# define HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_

#include "Manager.hpp"

#include <exception>
#include <stdexcept>
#include <atomic>

#include "Connection.hpp"

static std::once_flag curl_init_flag;
static void init_curl()
{
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        throw std::runtime_error("Cannot initialize curl");
    }
}

namespace HTTPP
{
namespace HTTP
{
namespace client
{
namespace detail
{

Manager::Manager(UTILS::ThreadPool& pool)
: handler(nullptr)
, pool(pool)
, strand(pool.getService())
, timer(pool.getService())
{
    std::call_once(curl_init_flag, &init_curl);
    handler = curl_multi_init();
    if (!handler)
    {
        throw std::runtime_error("Cannot initialize curl multi handle");
    }

    manager_setopt(CURLMOPT_SOCKETFUNCTION, &sock_cb);
    manager_setopt(CURLMOPT_SOCKETDATA, this);
    manager_setopt(CURLMOPT_TIMERFUNCTION, &curl_timer_cb);
    manager_setopt(CURLMOPT_TIMERDATA, this);
}

Manager::~Manager()
{
    if (handler)
    {
        curl_multi_cleanup(handler);
    }
}

void Manager::timer_cb(const boost::system::error_code& error)
{
    if (!error)
    {
        int still_running = 0;
        auto rc = curl_multi_socket_action(
            handler, CURL_SOCKET_TIMEOUT, 0, &still_running);

        if (rc != CURLM_OK)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error curl multi: " << curl_multi_strerror(rc);
            throw std::runtime_error("timer_cb error");
        }

        checkHandles();
    }
}

int Manager::curl_timer_cb(CURLM*, long timeout_ms, void* userdata)
{
    Manager* manager = (Manager*)userdata;

    manager->timer.cancel();

    if (timeout_ms > 0)
    {
        manager->timer.expires_from_now(boost::posix_time::millisec(timeout_ms));
        manager->timer.async_wait(
            std::bind(&Manager::timer_cb, manager, std::placeholders::_1));
    }
    else
    {
        boost::system::error_code error; /*success*/
        manager->timer_cb(error);
    }

    return 0;
}

int Manager::sock_cb(CURL* easy,
                     curl_socket_t s,
                     int what,
                     void* multi_private,
                     void* socket_private)
{
    Manager* manager = (Manager*)multi_private;

    if (what == CURL_POLL_REMOVE)
    {
        return 0;
    }

    if (!socket_private)
    {
        void* v = nullptr;
        auto rc = curl_easy_getinfo(easy, CURLINFO_PRIVATE, &v);
        if (rc != CURLE_OK)
        {
            throw std::runtime_error("Cannot get private info:" +
                                     std::string(curl_easy_strerror(rc)));
        }

        curl_multi_assign(manager->handler, s, v);
        socket_private = v;
    }

    Connection* connection = (Connection*)socket_private;
    manager->poll(connection, what);
    return 0;
}

void Manager::checkHandles()
{
    CURLMsg* msg;
    int msgs_left = 0;
    while ((msg = curl_multi_info_read(handler, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            auto easy = msg->easy_handle;
            auto result = msg->data.result;

            Connection* conn;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
            auto rc = curl_multi_remove_handle(handler, easy);

            if (rc != CURLM_OK)
            {
                BOOST_LOG_TRIVIAL(error) << "Cannot unregister easy handle";
            }

            pool.post([result, conn, easy, this]
                      { conn->buildResponse(result); });
        }
    }
}

void Manager::performOp(Connection* connection, int action)
{
    int still_running = 0;
    auto rc = curl_multi_socket_action(
        handler, connection->socket.native_handle(), action, &still_running);
    if (rc != CURLM_OK)
    {
        throw std::runtime_error(curl_multi_strerror(rc));
    }

    checkHandles();

    if (still_running <= 0)
    {
        timer.cancel();
    }
}

void Manager::poll(Connection* connection, int action)
{
    boost::asio::ip::tcp::socket& tcp_socket = connection->socket;

    if (action == CURL_POLL_IN)
    {
        tcp_socket.async_read_some(
            boost::asio::null_buffers(),
            strand.wrap(std::bind(&Manager::performOp, this, connection, action)));
    }
    else if (action == CURL_POLL_OUT)
    {
        tcp_socket.async_write_some(
            boost::asio::null_buffers(),
            strand.wrap(std::bind(&Manager::performOp, this, connection, action)));
    }
    else if (action == CURL_POLL_INOUT)
    {
        tcp_socket.async_read_some(
            boost::asio::null_buffers(),
            strand.wrap(std::bind(&Manager::performOp, this, connection, action)));

        tcp_socket.async_write_some(
            boost::asio::null_buffers(),
            strand.wrap(std::bind(&Manager::performOp, this, connection, action)));
    }
}

void Manager::handleRequest(Method method, Connection::ConnectionPtr connection)
{

    Connection* conn = connection.release();

    strand.post([this, method, conn]()
                {
                    try
                    {
                        conn->configureRequest(method);
                    }
                    catch (...)
                    {
                        conn->complete(std::current_exception());
                        return;
                    }

                    auto rc = curl_multi_add_handle(handler, conn->handle);
                    if (rc != CURLM_OK)
                    {
                        std::string message = curl_multi_strerror(rc);
                        BOOST_LOG_TRIVIAL(error)
                            << "Error scheduling a new request: " << message;
                        conn->complete(std::make_exception_ptr(
                            std::runtime_error(message)));
                    }
                });
}

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP
#endif // !HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_

