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
#include <iostream>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static const std::map<std::string, std::string> map = {
    { "toto", "kiki" }, { "bad", "12^%$@#1245&/?\\" }
};

void handler(Connection* connection, Request&& request)
{
    BOOST_CHECK_EQUAL(request.uri, "/test/kiki/");

    auto const& query_params = request.query_params;
    for (const auto& q : query_params)
    {
        BOOST_CHECK_EQUAL(map.at(q.first), q.second);
    }

    BOOST_CHECK_EQUAL(query_params.size(), map.size());


    connection->response().setCode(HTTPP::HTTP::HttpCode::Ok);
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(reuse_connection)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .joinUrlPath("test")
        .joinUrlPath("kiki", true);

    for (const auto& kv : map)
    {
        request.addUrlVariable(kv.first, kv.second);
    }

    for (int i = 0; i < 5; ++i)
    {
        auto response = client.get(std::move(request));
        BOOST_CHECK(response.code == HTTP::HttpCode::Ok);
        request = std::move(response.request);
    }
}
