/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "Manager.hpp"

#include <exception>
#include <stdexcept>
#include <atomic>
#include <mutex>
#include <functional>

#include "Connection.hpp"

static std::once_flag curl_init_flag;
static void init_curl()
{
    BOOST_LOG_TRIVIAL(info) << "initialize libcurl";
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        throw std::runtime_error("Cannot initialize curl");
    }

    ::atexit(&::curl_global_cleanup);

    BOOST_LOG_TRIVIAL(info) << "curl initialized";
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
    auto conns = std::move(current_connections);
    for (auto conn : conns)
    {
        removeConnection(conn);
    }

    if (handler)
    {
        curl_multi_cleanup(handler);
    }
    timer.cancel();
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

    if (timeout_ms > 0)
    {
        manager->handleCancelledConnections();
        manager->timer.expires_from_now(boost::posix_time::millisec(timeout_ms));
        manager->timer.async_wait(manager->strand.wrap(
            std::bind(&Manager::timer_cb, manager, std::placeholders::_1)));
    }
    else
    {
        manager->timer.cancel();
        manager->strand.post([manager]
                             {
                                 boost::system::error_code error; /*success*/
                                 manager->timer_cb(error);
                             });
    }

    return 0;
}

int Manager::sock_cb(CURL* easy,
                     curl_socket_t s,
                     int what,
                     void* multi_private,
                     void*)
{
    Manager* manager = (Manager*)multi_private;

    if (what == CURL_POLL_REMOVE)
    {
        return 0;
    }

    Connection* connection;
    auto rc = curl_easy_getinfo(easy, CURLINFO_PRIVATE, &connection);
    if (rc != CURLE_OK)
    {
        throw std::runtime_error("Cannot get private info:" +
                                 std::string(curl_easy_strerror(rc)));
    }

    connection->setSocket(s);
    connection->poll_action = what;
    manager->poll(connection->shared_from_this(), what);
    return 0;
}

void Manager::removeConnection(std::shared_ptr<Connection> conn)
{
    auto rc = curl_multi_remove_handle(handler, conn->handle);

    if (rc != CURLM_OK)
    {
        BOOST_LOG_TRIVIAL(error)
            << "Cannot unregister easy handle: " << curl_multi_strerror(rc);
    }

    current_connections.erase(std::find(std::begin(current_connections),
                                        std::end(current_connections),
                                        conn),
                              std::end(current_connections));
}

void Manager::removeConnection(Connection* conn)
{
    removeConnection(conn->shared_from_this());
}

void Manager::removeConnection(CURL* easy)
{
    Connection* conn;
    curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
    removeConnection(conn);
}

void Manager::handleCancelledConnections()
{
    decltype(current_connections) copy;
    copy.swap(current_connections);

    for (auto conn : copy)
    {
        if (conn->is_canceled)
        {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled operation detected";
            removeConnection(conn);
        }
        else
        {
            current_connections.emplace_back(conn);
        }
    }
}

void Manager::checkHandles()
{
    handleCancelledConnections();

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
            conn->poll_action = 0;

            auto shared_ptr = conn->shared_from_this();
            removeConnection(shared_ptr);


            pool.post([result, shared_ptr, easy, this]
                      { shared_ptr->buildResponse(result); });
        }
    }
}

void Manager::performOp(std::shared_ptr<Connection> connection, int action)
{
    int still_running = 0;

    connection->is_polling = false;

    if (connection->is_canceled)
    {
        BOOST_LOG_TRIVIAL(debug) << "Cancelled operation detected";
        removeConnection(connection);
        return;
    }

    auto rc = curl_multi_socket_action(
        handler, connection->socket->native_handle(), action, &still_running);
    if (rc != CURLM_OK)
    {
        auto exc = std::make_exception_ptr(std::runtime_error(curl_multi_strerror(rc)));
        connection->complete(exc);
        std::rethrow_exception(exc);
    }

    checkHandles();

    if (still_running <= 0)
    {
        timer.cancel();
    }
    else
    {
        if (action == connection->poll_action)
        {
            poll(connection, connection->poll_action);
        }
    }
}

void Manager::poll(std::shared_ptr<Connection> connection, int action)
{
    connection->is_polling = true;

    if (connection->is_canceled)
    {
        BOOST_LOG_TRIVIAL(debug) << "Cancelled operation detected";
        removeConnection(connection);
        return;
    }

    boost::asio::ip::tcp::socket& tcp_socket = *connection->socket;

    if (action == CURL_POLL_IN)
    {
        tcp_socket.async_read_some(boost::asio::null_buffers(),
                                   strand.wrap(std::bind(&Manager::performOp,
                                                         this,
                                                         connection,
                                                         action)));
    }
    else if (action == CURL_POLL_OUT)
    {
        tcp_socket.async_write_some(boost::asio::null_buffers(),
                                    strand.wrap(std::bind(&Manager::performOp,
                                                          this,
                                                          connection,
                                                          action)));
    }
    else if (action == CURL_POLL_INOUT)
    {
        tcp_socket.async_read_some(boost::asio::null_buffers(),
                                   strand.wrap(std::bind(&Manager::performOp,
                                                         this,
                                                         connection,
                                                         action)));

        tcp_socket.async_write_some(boost::asio::null_buffers(),
                                    strand.wrap(std::bind(&Manager::performOp,
                                                          this,
                                                          connection,
                                                          action)));
    }
    else
    {
        connection->complete(std::make_exception_ptr(
            std::runtime_error("Unknow poll operation requested")));
    }
}

void Manager::handleRequest(Method method, Connection::ConnectionPtr conn)
{

    strand.post([this, method, conn]()
                {
                    try
                    {
                        conn->configureRequest(method);
                    }
                    catch (const std::exception& exc)
                    {
                        BOOST_LOG_TRIVIAL(error)
                            << "Error when configuring the request: "
                            << exc.what();
                        conn->complete(std::current_exception());
                        return;
                    }

                    current_connections.emplace_back(conn);

                    auto rc = curl_multi_add_handle(handler, conn->handle);
                    if (rc != CURLM_OK)
                    {
                        std::string message = curl_multi_strerror(rc);
                        BOOST_LOG_TRIVIAL(error)
                            << "Error scheduling a new request: " << message;
                        removeConnection(conn);
                        conn->complete(std::make_exception_ptr(
                            std::runtime_error(message)));
                        return;
                    }

                });
}

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP
