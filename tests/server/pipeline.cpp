/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <istream>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static const std::string REQUEST = "GET / HTTP/1.1\r\n\r\n"
                                   "GET / HTTP/1.1\r\n\r\n"
                                   "GET / HTTP/1.1\r\n\r\n";

static const std::string BODY(8192, 'a');

static const std::string REQUEST_BODY_ = "GET / HTTP/1.1\r\n"
                                         "Content-Length: 8192\r\n"
                                         "\r\n" +
                                         BODY;
static const std::string REQUEST_BODY =
    REQUEST_BODY_ + REQUEST_BODY_ + REQUEST_BODY_;

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
    boost::asio::connect(s, resolver.resolve({ "localhost", "8000" }));
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

            //std::cout << i << std::endl;
        } while (not is.eof() || i < 3);
    }
}

size_t total_size = 0;
void body_handler(const Request&,
                  Connection* connection,
                  const boost::system::error_code& ec,
                  const char*,
                  size_t n)
{
    if (ec == boost::asio::error::eof)
    {
        connection->response().setCode(HTTP::HttpCode::Ok).setBody("");
        connection->sendResponse(); // connection pointer may become invalid
    }
    else if (ec)
    {
        throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
    }
    else
    {
        total_size += n;
    }
}

void handler_w_body(Connection* connection)
{
    auto& request = connection->request();
    auto headers = request.getSortedHeaders();
    auto const& content_length = headers["Content-Length"];
    auto size = std::stoi(to_string(content_length));
    connection->readBody(size, std::bind(&body_handler, std::cref(request),
                                         connection, std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3));
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
    boost::asio::connect(s, resolver.resolve({ "localhost", "8000" }));
    boost::asio::write(s, boost::asio::buffer(REQUEST_BODY));
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

            //std::cout << i << std::endl;
        } while (not is.eof() || i < 3);
    }

    BOOST_CHECK_EQUAL(total_size, BODY.size() * 3);
}

