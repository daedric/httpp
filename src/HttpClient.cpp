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

void HttpClient::handleRequest(HTTP::Method method,
                               Request&& request,
                               CompletionHandler&& completion_handler)
{
    Connection::ConnectionPtr connection;

    if (request.connection_)
    {
        connection = std::move(request.connection_);
    }
    else
    {
        connection = Connection::createConnection(service_);
    }

    connection->init();
    connection->request = std::move(request);
    connection->completion_handler = std::move(completion_handler);
    manager->handleRequest(method, std::move(connection));
}

HttpClient::Future HttpClient::handleRequest(HTTP::Method method, Request&& request)
{
    Connection::ConnectionPtr connection;

    if (request.connection_)
    {
        connection = std::move(request.connection_);
    }
    else
    {
        connection = Connection::createConnection(service_);
    }

    connection->init();
    connection->request = std::move(request);
    auto future = connection->promise.get_future();

    manager->handleRequest(method, std::move(connection));
    return future;
}

#define METHOD_post POST
#define METHOD_get GET
#define METHOD_head HEAD
#define METHOD_put PUT
#define METHOD_delete_ DELETE_
#define METHOD_options OPTIONS
#define METHOD_trace TRACE
#define METHOD_connect CONNECT

#define METHOD(m)                                                       \
    HttpClient::Future HttpClient::async_##m(Request&& req)             \
    {                                                                   \
        return handleRequest(HTTP::Method::METHOD_##m, std::move(req)); \
    }                                                                   \
    HttpClient::Response HttpClient::m(Request&& request)               \
    {                                                                   \
        auto fut = async_##m(std::move(request));                       \
        if (fut.valid())                                                \
        {                                                               \
            return fut.get();                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            throw std::runtime_error("invalid future");                 \
        }                                                               \
    }                                                                   \
    void HttpClient::async_##m(Request&& request,                       \
                               CompletionHandler&& completion_handler)  \
    {                                                                   \
        handleRequest(HTTP::Method::METHOD_##m,                         \
                      std::move(request),                               \
                      std::move(completion_handler));                   \
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
