/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

void handler(Connection* connection, Request&&)
{
    Connection::release(connection);
}

BOOST_AUTO_TEST_CASE(wild_disconnection)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");
    BOOST_CHECK_THROW(client.get(std::move(request)), std::exception);
}

BOOST_AUTO_TEST_CASE(no_server)
{
    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");
    BOOST_CHECK_THROW(client.get(std::move(request)), std::exception);
}

