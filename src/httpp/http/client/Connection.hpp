/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_
#define HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_

#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>
#include <curl/curl.h>

#include <commonpp/core/LoggingInterface.hpp>
#include <commonpp/thread/ThreadPool.hpp>

#include "httpp/detail/config.hpp"
#include "httpp/http/Protocol.hpp"
#include "httpp/http/client/Request.hpp"
#include "httpp/http/client/Response.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace client
{

void parseCurlResponseHeader(const std::vector<char>& headers, Response& response);

namespace detail
{

struct Manager;

FWD_DECLARE_LOGGER(client_connection_logger, commonpp::core::Logger);

struct Connection : public std::enable_shared_from_this<Connection>
{
    using ConnectionPtr = std::shared_ptr<Connection>;
    using ThreadPool = commonpp::thread::ThreadPool;

    template <typename T>
    using Promise = HTTPP::detail::Promise<T>;

    template <typename T>
    using Future = HTTPP::detail::Future<T>;

    using ExceptionPtr = HTTPP::detail::ExceptionPtr;

    using CompletionHandler = std::function<void(Future<Response>&&)>;

    Connection(Manager& manager, boost::asio::io_service& service);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    ~Connection();

    template <typename T>
    void conn_setopt(CURLoption opt, T t)
    {
        auto rc = curl_easy_setopt(handle, opt, t);
        if (rc != CURLE_OK)
        {
            LOG(client_connection_logger, error)
                << "Error setting curl option: " << curl_easy_strerror(rc);
            throw std::runtime_error("Cannot set option on curl");
        }
    }

    template <typename T>
    T conn_getinfo(CURLINFO info)
    {
        T data;
        auto rc = curl_easy_getinfo(handle, info, std::addressof(data));
        if (rc != CURLE_OK)
        {
            LOG(client_connection_logger, error)
                << "Can't get info: " << curl_easy_strerror(rc);
            throw std::runtime_error(curl_easy_strerror(rc));
        }

        return data;
    }

    void init(std::map<curl_socket_t, boost::asio::ip::tcp::socket*>& sockets);
    static ConnectionPtr createConnection(Manager& manager,
                                          boost::asio::io_service& service);

    static size_t writefn(char* buffer, size_t size, size_t nmemb, void* userdata);
    static size_t writeHd(char* buffer, size_t size, size_t nmemb, void* userdata);
    static curl_socket_t opensocket(void* clientp,
                                    curlsocktype purpose,
                                    struct curl_sockaddr* address);

    static int closesocket(void* clientp, curl_socket_t socket);

    void configureRequest(HTTPP::HTTP::Method method);
    void cancel();

    void buildResponse(CURLcode code);
    void complete(ExceptionPtr ex = ExceptionPtr());
    void setSocket(curl_socket_t socket);

    template <typename Cb>
    void poll(int action, Cb cb)
    {
        LOG(client_connection_logger, trace)
            << "Poll socket: " << socket
            << ", socket native_handle: " << socket->native_handle();

        switch (action)
        {
        default:
            LOG(client_connection_logger, error)
                << "Unknow poll operation requested: " << action;

            complete(HTTPP::detail::make_exception_ptr(
                std::runtime_error("Unknow poll operation requested")));
            break;
        case CURL_POLL_IN:
            socket->async_read_some(boost::asio::null_buffers(), cb);
            break;
        case CURL_POLL_OUT:
            socket->async_write_some(boost::asio::null_buffers(), cb);
            break;
        case CURL_POLL_INOUT:
            socket->async_read_some(boost::asio::null_buffers(), cb);
            socket->async_write_some(boost::asio::null_buffers(), cb);
            break;
        }
    }

    void cancelPoll()
    {
        if (socket)
        {
            socket->cancel();
        }
    }

    Manager& handler;
    ThreadPool* dispatch = nullptr;

    CURL* handle = nullptr;
    int poll_action = 0;
    std::atomic_bool cancelled = {false};
    char error_buffer[CURL_ERROR_SIZE] = {0};

    boost::asio::io_service& service;
    boost::asio::ip::tcp::socket* socket = nullptr;
    std::map<curl_socket_t, boost::asio::ip::tcp::socket*>* sockets = nullptr;

    client::Request request;
    client::Response response;

    Promise<client::Response> promise;
    CompletionHandler completion_handler;
    bool expect_continue = false;
    std::vector<char> header;
    std::vector<char> buffer;
    std::atomic_bool result_notified = {true};

    struct curl_slist* http_headers = nullptr;
};

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // !HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_
