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

static Connection* gconnection = nullptr;
void body_handler(const boost::system::error_code& ec, const char* buffer, size_t n)
{
    static std::string body_read;

    std::cout.write(buffer, n) << std::endl;
    if (ec == boost::asio::error::eof)
    {
        std::cout << "Body read: " << body_read << std::endl;
        (gconnection->response() = Response(HTTP::HttpCode::Ok))
            .connectionShouldBeClosed(true);
        gconnection->sendResponse();
    }
    else if (ec)
    {
        throw ec;
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
    auto size = std::stoi(headers["Content-Length"]);
    std::cout << "Read a size of : " << size << std::endl;
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
