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
# include <future>

# include <boost/asio.hpp>

# include "utils/ThreadPool.hpp"
# include "http/Protocol.hpp"
# include "http/client/Request.hpp"
# include "http/client/Response.hpp"

namespace HTTPP
{

class HttpClient
{
    struct Manager;

public:
    using Request = HTTP::client::Request;
    using Response = HTTP::client::Response;
    using Future = std::future<Response>;

    HttpClient(size_t nb_thread = 2);
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    ~HttpClient();

#define METHOD(m)                  \
    Response m(Request&& request); \
    Future async_ ## m(Request&& request);

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
    Future handleRequest(HTTP::Method method, Request&& request);

private:
    boost::asio::io_service service_;
    UTILS::ThreadPool pool_;
    std::unique_ptr<Manager> manager;
};

} // namespace HTTPP
#endif // ! _HTTPP__HTTP_CLIEN_HPP_
