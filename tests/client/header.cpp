/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <boost/test/unit_test.hpp>

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

void handler(Connection* connection)
{
    auto& request = connection->request();
    auto headers = request.getSortedHeaders();

    BOOST_CHECK_EQUAL(headers["test"], "toto");
    BOOST_CHECK_EQUAL(headers["titi"], ":lol:");

    connection->response()
        .setCode(HTTP::HttpCode::NoContent)
        .setBody("Ok")
        .addHeader("hello", "world")
        .addHeader("testy", "lol")
        .addHeader("coucou", "salut")
        .connectionShouldBeClosed(true);
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(send_and_parse_header)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080")
        .addHeader("test", "toto")
        .addHeader("titi", ":lol:")
        .followRedirect(true);

    auto response = client.get(std::move(request));
    auto headers = response.getSortedHeaders();
    BOOST_CHECK_EQUAL(headers["hello"], "world");
    BOOST_CHECK_EQUAL(headers["testy"], "lol");
    BOOST_CHECK_EQUAL(headers["coucou"], "salut");
}
