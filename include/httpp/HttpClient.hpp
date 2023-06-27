/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_CLIENT_HPP_
#define _HTTPP__HTTP_CLIENT_HPP_

#include <memory>
#include <string>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <commonpp/thread/ThreadPool.hpp>

#include "detail/config.hpp"
#include "http/Protocol.hpp"
#include "http/client/Request.hpp"
#include "http/client/Response.hpp"
#include "utils/Exception.hpp"

namespace HTTPP
{

namespace HTTP
{
namespace client
{
namespace detail
{
struct Manager;
struct Connection;
} // namespace detail
} // namespace client
} // namespace HTTP

class HttpClient
{
public:
    using ThreadPool = commonpp::thread::ThreadPool;

    using Request = HTTP::client::Request;
    using Response = HTTP::client::Response;

    using Future = detail::Future<Response>;
    using CompletionHandler = std::function<void(Future)>;

    // AsyncHandler is garanteed to be always safe to call
    class AsyncHandler
    {
        friend class HttpClient;

    public:
        void cancelOperation();

    private:
        std::weak_ptr<HTTP::client::detail::Connection> connection_;
    };

    HttpClient(size_t nb_thread = 1, const std::string& name = "");
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    ~HttpClient();

#define METHOD(m)                                                              \
    Response m(Request request);                                               \
    Future async_##m(Request request);                                         \
    AsyncHandler async_##m(Request request, CompletionHandler);

    METHOD(post);
    METHOD(get);
    METHOD(head);
    METHOD(put);
    METHOD(delete_);
    METHOD(options);
    METHOD(trace);
    METHOD(connect);

#undef METHOD

private:
    std::pair<Future, AsyncHandler> handle_request(
        HTTP::Method method, Request request, CompletionHandler handler = CompletionHandler()
    );

private:
    ThreadPool pool_io_;
    ThreadPool pool_dispatch_;
    std::unique_ptr<HTTP::client::detail::Manager> manager;
};

} // namespace HTTPP
#endif // ! _HTTPP__HTTP_CLIEN_HPP_
