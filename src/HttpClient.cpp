/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/HttpClient.hpp"

#include <iostream>
#include <mutex>
#include <stdexcept>

#include "http/client/Manager.hpp"
#include "http/client/Connection.hpp"

namespace HTTPP
{

using HTTP::client::detail::Connection;
using HTTP::client::detail::Manager;

void HttpClient::AsyncHandler::cancelOperation()
{
    auto ptr = connection_.lock();
    if (ptr)
    {
        auto future = ptr->cancel_promise.get_future();
        ptr->cancel();
        ptr.reset();
        future.get();
    }
}

HttpClient::HttpClient(size_t nb_thread)
: pool_(nb_thread, service_, "HttpClient ThreadPool")
, manager(new Manager(pool_))
{
    pool_.start();
}

HttpClient::~HttpClient()
{
    pool_.stop();
}

std::pair<HttpClient::Future, HttpClient::AsyncHandler>
HttpClient::handle_request(HTTP::Method method,
                           Request&& request,
                           CompletionHandler completion_handler)
{
    Connection::ConnectionPtr connection;

    if (request.connection_)
    {
        connection = std::move(request.connection_);
        if (connection.use_count() != 1)
        {
            connection.reset(); // avoid any problem
        }
    }

    if (!connection)
    {
        connection = Connection::createConnection(service_);
    }

    connection->init(manager->sockets);
    connection->request = std::move(request);

    Future fut;
    AsyncHandler hndl;

    if (completion_handler)
    {
        connection->completion_handler = std::move(completion_handler);
        hndl.connection_ = connection->shared_from_this();
    }
    else
    {
        fut = std::move(connection->promise.get_future());
    }

    manager->handleRequest(method, std::move(connection));
    return std::make_pair(std::move(fut), hndl);
}


#define METHOD_post POST
#define METHOD_get GET
#define METHOD_head HEAD
#define METHOD_put PUT
#define METHOD_delete_ DELETE_
#define METHOD_options OPTIONS
#define METHOD_trace TRACE
#define METHOD_connect CONNECT

#define METHOD(m)                                                              \
    HttpClient::Future HttpClient::async_##m(Request&& req)                    \
    {                                                                          \
        return handle_request(HTTP::Method::METHOD_##m, std::move(req)).first; \
    }                                                                          \
    HttpClient::Response HttpClient::m(Request&& request)                      \
    {                                                                          \
        Future fut = async_##m(std::move(request));                            \
                                                                               \
        if (fut.valid())                                                       \
        {                                                                      \
            return fut.get();                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            throw std::runtime_error("invalid future");                        \
        }                                                                      \
    }                                                                          \
    HttpClient::AsyncHandler HttpClient::async_##m(                            \
        Request&& request, CompletionHandler&& completion_handler)             \
    {                                                                          \
        return handle_request(HTTP::Method::METHOD_##m,                        \
                              std::move(request),                              \
                              std::move(completion_handler)).second;           \
    }

METHOD(post);
METHOD(get);
METHOD(head);
METHOD(put);
METHOD(delete_);
METHOD(options);
METHOD(trace);
METHOD(connect);

} // namespace HTTPP
