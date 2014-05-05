/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_CLIENT_HPP_
# define _HTTPP__HTTP_CLIENT_HPP_

# include <string>
# include <vector>
# include <memory>

# include <boost/asio/io_service.hpp>

# include "detail/config.hpp"
# include "utils/Exception.hpp"
# include "utils/ThreadPool.hpp"
# include "http/Protocol.hpp"
# include "http/client/Request.hpp"
# include "http/client/Response.hpp"

namespace HTTPP
{

namespace HTTP { namespace client { namespace detail {
struct Manager;
struct Connection;
} } }

class HttpClient
{

public:
    using Request = HTTP::client::Request;
    using Response = HTTP::client::Response;

    using Future = detail::Future<Response>;
    using CompletionHandler = std::function<void (Future&&)>;

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

#define METHOD(m)                        \
    Response m(Request&& request);       \
    Future async_##m(Request&& request); \
    AsyncHandler async_##m(Request&& request, CompletionHandler&&);

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
    std::pair<Future, AsyncHandler>
    handle_request(HTTP::Method method,
                   Request&& request,
                   CompletionHandler handler = CompletionHandler());

private:
    UTILS::ThreadPool pool_io_;
    UTILS::ThreadPool pool_dispatch_;
    std::unique_ptr<HTTP::client::detail::Manager> manager;
};

} // namespace HTTPP
#endif // ! _HTTPP__HTTP_CLIEN_HPP_
