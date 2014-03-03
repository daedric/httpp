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

void handler(Connection*, Request&&)
{
}

BOOST_AUTO_TEST_CASE(cancel_async_operation)
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

    auto handler = client.async_get(
        std::move(request),
        [](HttpClient::Future&& fut)
        { BOOST_CHECK_THROW(fut.get(), HTTPP::UTILS::OperationAborted); });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    handler.cancelOperation();
    BOOST_LOG_TRIVIAL(error) << "operation cancelled";
}

