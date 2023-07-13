/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */

#include <chrono>
#include <iostream>
#include <istream>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/http/Connection.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

static const std::string REQUEST =
    "GET / HTTP/1.1\r\n\r\n"
    "GET / HTTP/1.1\r\n\r\n"
    "GET / HTTP/1.1\r\n\r\n";

static const std::string BODY = []
{
    std::string s;
    s.reserve(8192);
    for (char a = 'a'; a < 'z'; a++)
    {
        s.append(256, a);
    }

    for (char a = 'A'; a < 'Z' && s.size() != 8192; a++)
    {
        s.append(256, a);
    }

    return s;
}();

static const std::string REQUEST_BODY_ =
    "GET / HTTP/1.1\r\n"
    "Content-Length: 8192\r\n"
    "\r\n"
    + BODY;
static const std::string REQUEST_BODY = REQUEST_BODY_ + REQUEST_BODY_ + REQUEST_BODY_;

void handler(Connection* connection)
{
    connection->response().setCode(HTTP::HttpCode::Ok).setBody("");
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(pipeline)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    boost::asio::write(s, boost::asio::buffer(REQUEST));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int i = 0;
    while (i < 3)
    {
        boost::asio::streambuf b;
        boost::asio::read_until(s, b, "\r\n");
        std::istream is(&b);
        do
        {
            std::string line;
            std::getline(is, line);
            boost::trim(line);

            if (line == "HTTP/1.1 200 Ok")
            {
                ++i;
            }

            // std::cout << i << std::endl;
        } while (not is.eof() || i < 3);
    }
}

size_t total_size = 0;
std::atomic_int r = 0;

void handler_w_body(Connection* connection)
{
    if (r++ == 0)
    {
        read_whole_request(
            connection,
            [](std::unique_ptr<HTTP::helper::ReadWholeRequest> hndl,
               const boost::system::error_code& ec)
            {
                if (ec)
                {
                    throw UTILS::convert_boost_ec_to_std_ec(ec);
                }
                BOOST_REQUIRE_EQUAL(
                    std::string_view(hndl->body.data(), hndl->body.size()), BODY
                );
                total_size += hndl->body.size();
                hndl->connection->response()
                    .setCode(HTTP::HttpCode::Ok)
                    .setBody("");
                hndl->connection->sendResponse();
            }
        );
        return;
    }

    connection->read_whole(
        BODY.size(),
        [connection](const boost::system::error_code& ec)
        {
            if (ec)
            {
                throw UTILS::convert_boost_ec_to_std_ec(ec);
            }

            auto [ptr, sz] = connection->mutable_body();
            BOOST_REQUIRE_EQUAL(sz, BODY.size());
            BOOST_REQUIRE_EQUAL(std::string_view(ptr, sz), BODY);

            total_size += sz;
            connection->response().setCode(HTTP::HttpCode::Ok).setBody("");
            connection->sendResponse();
        }
    );
}

BOOST_AUTO_TEST_CASE(pipeline_with_body)
{
    HttpServer server;
    server.start();
    server.setSink(&handler_w_body);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    auto n = boost::asio::write(s, boost::asio::buffer(REQUEST_BODY));
    BOOST_REQUIRE_EQUAL(n, REQUEST_BODY.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int i = 0;
    while (i < 3)
    {
        boost::asio::streambuf b;
        boost::asio::read_until(s, b, "\r\n");
        std::istream is(&b);
        do
        {
            std::string line;
            std::getline(is, line);
            boost::trim(line);

            if (line == "HTTP/1.1 200 Ok")
            {
                ++i;
            }

            // std::cout << i << std::endl;
        } while (not is.eof() || i < 3);
    }

    BOOST_CHECK_EQUAL(total_size, BODY.size() * 3);
}
