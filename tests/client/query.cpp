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

#include <boost/test/unit_test.hpp>

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"
#include "httpp/http/Connection.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

static const std::map<std::string, std::string> map = {
    {"toto", "kiki"}, {"bad", "12^%$@#1245&/?\\"}};

void handler(Connection* connection)
{
    auto& request = connection->request();
    BOOST_CHECK_EQUAL(request.uri, "/test/kiki/");

    const auto& query_params = request.query_params;
    for (const auto& q : query_params)
    {
        BOOST_CHECK_EQUAL(map.at(q.first), q.second);
    }

    BOOST_CHECK_EQUAL(query_params.size(), map.size());

    connection->response().setCode(HTTPP::HTTP::HttpCode::Ok);
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(query_params)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080").joinUrlPath("test").joinUrlPath("kiki", true);

    for (const auto& kv : map)
    {
        request.addUrlVariable(kv.first, kv.second);
    }

    client.get(std::move(request));
}
