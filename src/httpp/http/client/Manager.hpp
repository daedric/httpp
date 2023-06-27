/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_
#define HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_

#include <future>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>

#include <boost/asio.hpp>
#include <commonpp/core/LoggingInterface.hpp>
#include <commonpp/thread/ThreadPool.hpp>

#include "httpp/detail/config.hpp"
#include "httpp/http/Protocol.hpp"
#include <curl/curl.h>

namespace HTTPP
{
namespace HTTP
{
namespace client
{

namespace detail
{

struct Connection;

FWD_DECLARE_LOGGER(manager_logger, commonpp::core::Logger);

struct Manager
{
    using ThreadPool = commonpp::thread::ThreadPool;
    using Method = HTTPP::HTTP::Method;
    using ConnectionPtr = std::shared_ptr<Connection>;

    template <typename T>
    using Promise = HTTPP::detail::Promise<T>;

    template <typename T>
    using Future = HTTPP::detail::Future<T>;

    Manager(ThreadPool& io, ThreadPool& dispatch);
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    ~Manager();

    template <typename T>
    void manager_setopt(CURLMoption opt, T t)
    {
        auto rc = curl_multi_setopt(handler, opt, t);
        if (rc != CURLM_OK)
        {
            LOG(manager_logger, error)
                << "Error setting curl option: " << curl_multi_strerror(rc);
            throw std::runtime_error("Cannot set option on curl");
        }
    }

    void timer_cb(const boost::system::error_code& error);
    static int curl_timer_cb(CURLM*, long timeout_ms, void* userdata);

    static int
    sock_cb(CURL* easy, curl_socket_t s, int what, void* multi_private, void* socket_private);

    void checkHandles();
    void performOp(
        std::shared_ptr<Connection> connection, int action, const boost::system::error_code& ec
    );
    void poll(std::shared_ptr<Connection> connection, int action);

    void cancelConnection(std::shared_ptr<Connection> connection);
    Future<void> cancel_connection(std::shared_ptr<Connection> connection);
    void cancel_connection_io_thread(
        std::shared_ptr<Connection> connection, std::shared_ptr<Promise<void>> promise
    );

    int closeSocket(curl_socket_t curl_socket);

    void removeHandle(CURL* easy);
    void removeConnection(std::shared_ptr<Connection> conn);

    void handleRequest(Method method, ConnectionPtr connection);

    std::map<curl_socket_t, boost::asio::ip::tcp::socket*> sockets;

    bool running = true;
    CURLM* handler;
    ThreadPool& io;
    ThreadPool& dispatch;

    using Timer = boost::asio::deadline_timer;
    Timer timer;

    enum State
    {
        Default,
        Polling,
        PerformIo,
        Cancelled,
    };

    std::map<std::shared_ptr<Connection>, State> current_connections;
    std::map<std::shared_ptr<Connection>, Promise<void>> cancelled_connections;
};
} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // ! HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_
