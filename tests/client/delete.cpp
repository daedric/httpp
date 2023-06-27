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

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"
#include "httpp/http/Connection.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

void handler(Connection* connection)
{
    connection->response().setCode(HTTP::HttpCode::Ok);
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(delete_test)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient::Request request;
    request.url("http://localhost:8080");

    HttpClient client;
    HttpClient client2;

    auto r = client.get(std::move(request));
    r = client2.get(std::move(r.request));
}
