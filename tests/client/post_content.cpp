/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static const std::string EXPECTED_BODY = R"*(PREFIX ?= /usr/local

all:
	make -C build all

clean:
	make -C build clean

cmake:
	rm -rf build
	mkdir build
	cd build && cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

package:
	make -C build package

test:
	make -C build test

re : cmake all)*";

static Connection* gconnection = nullptr;
void body_handler(const boost::system::error_code& ec, const char* buffer, size_t n)
{
    static std::string body_read;

    if (ec == boost::asio::error::eof)
    {
        BOOST_CHECK_EQUAL(body_read, EXPECTED_BODY);
        (gconnection->response() = Response(HTTP::HttpCode::Ok))
            .setBody(EXPECTED_BODY)
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
    auto content_length = headers["Content-Length"];
    if (!content_length.empty())
    {
        auto size = std::stoi(headers["Content-Length"]);
        connection->readBody(size, &body_handler);
    } else {
        connection->response()
            .setCode(HTTP::HttpCode::BadRequest)
            .setBody("Expected body!");
        connection->sendResponse();
    }
}

BOOST_AUTO_TEST_CASE(post_content)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .setContent(EXPECTED_BODY);

    auto resp = client.post(std::move(request));
    std::string str(resp.body.data(), resp.body.size());

    BOOST_CHECK_EQUAL(EXPECTED_BODY, str);
}

