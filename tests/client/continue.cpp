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
        (gconnection->response() =
            Response(HTTP::HttpCode::Ok))
                .setBody("Body received");
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

void handler(Connection* connection)
{
    gconnection = connection;
    auto& request = connection->request();
    auto headers = request.getSortedHeaders();
    auto size = std::stoi(to_string(headers["Content-Length"]));
    if (headers["Expect"] == "100-continue")
    {
        connection->sendContinue([connection, size]
                                 { connection->readBody(size, &body_handler); });
    }
}

BOOST_AUTO_TEST_CASE(expect_continue)
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
        .pushPostData("Hello", "world")
        .pushPostData("test", "123!^&%@#^&%$");

    auto response = client.post(std::move(request));
    std::string body(response.body.data(), response.body.size());
    BOOST_CHECK_EQUAL(body, "Body received");
}
