/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>
#include <iostream>

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

void handler(Connection* connection)
{
    connection->response().setCode(HTTPP::HTTP::HttpCode::Ok).setBody("Hello world");
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(completion_handler)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");
    bool b = true;
    client.async_get(std::move(request), [&b](HttpClient::Future future) {
        auto response = future.get();
        std::string body(response.body.data(), response.body.size());
        BOOST_CHECK_EQUAL(body, "Hello world");
        b = false;
    });

    while (b)
    {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
