/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
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

void handler(Connection* connection)
{
    auto& request = connection->request();
    auto headers = request.getSortedHeaders();

    auto const& host = headers["Host"];
    auto port = stoi(to_string(host.substr(host.find(':') + 1)));

    if (port < 8084)
    {
        connection->response()
            .setCode(HTTP::HttpCode::MovedPermanently)
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
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::trace);
    commonpp::core::enable_console_logging();

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

void handler2(Connection* connection)
{
    auto& request = connection->request();
    if (request.uri == "//redirect")
    {
        connection->response()
            .setCode(HTTP::HttpCode::MovedPermanently)
            .addHeader("Location", "/ok")
            .setBody("");
        connection->sendResponse();
    }
    else if (request.uri == "/ok")
    {
        connection->response()
            .setCode(HTTP::HttpCode::Ok)
            .setBody("Ok");
        connection->sendResponse();
    }
    else
    {
        abort();
    }
}

BOOST_AUTO_TEST_CASE(follow_redirect2)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::warning);
    commonpp::core::enable_console_logging();

    HttpServer server;
    server.start();
    server.setSink(&handler2);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080//redirect")
        .followRedirect(true);

    auto response = client.get(std::move(request));
    BOOST_CHECK_EQUAL(std::string(response.body.data(), response.body.size()),
                      "Ok");
    for (const auto& h : response.headers)
    {
        std::cout << h.first << ": " << h.second << std::endl;
    }
}
