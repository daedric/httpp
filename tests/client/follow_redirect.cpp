/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <iostream>
#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

void handler(Connection* connection, Request&& request)
{

    auto headers = request.getSortedHeaders();

    auto const& host = headers["Host"];
    auto port = stoi(host.substr(host.find(':') + 1));

    if (port < 8084)
    {
        connection->response()
            .setCode(HTTP::HttpCode::MovedPermentaly)
            .addHeader("Location", "http://localhost:" + std::to_string(port + 1))
            .setBody("");
        connection->sendResponse();
    }
    else
    {
        connection->response()
            .setCode(HTTP::HttpCode::Ok)
            .setBody("Ok");
        connection->sendResponse();
    }
}

BOOST_AUTO_TEST_CASE(follow_redirect)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");
    server.bind("localhost", "8081");
    server.bind("localhost", "8082");
    server.bind("localhost", "8083");
    server.bind("localhost", "8084");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .followRedirect(true);

    auto response = client.get(std::move(request));
    BOOST_CHECK_EQUAL(std::string(response.body.data(), response.body.size()),
                      "Ok");
    for (const auto& h : response.headers)
    {
        std::cout << h.first << ": " << h.second << std::endl;
    }
}


