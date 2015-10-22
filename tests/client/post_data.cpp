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

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static Connection* gconnection = nullptr;
void body_handler(const boost::system::error_code& ec, const char* buffer, size_t n)
{
    static std::string body_read;

    if (ec == boost::asio::error::eof)
    {
        (gconnection->response() = Response(HTTP::HttpCode::Ok))
            .connectionShouldBeClosed(true);
        gconnection->sendResponse();
    }
    else if (ec)
    {
        throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
    }
    else
    {
        body_read.append(buffer, n);
    }
}

void handler(Connection* connection, Request&& request)
{
    gconnection = connection;
    auto headers = request.getSortedHeaders();
    auto size = std::stoi(to_string(headers["Content-Length"]));
    connection->readBody(size, &body_handler);
}

BOOST_AUTO_TEST_CASE(post_data)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    using PostEncoding = HttpClient::Request::PostEncoding;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .pushPostData("Hello", "world", PostEncoding::FormUrlEncoded)
        .pushPostData("test", "123!^&%@#^&%$", PostEncoding::FormUrlEncoded);

    client.post(std::move(request));
}
