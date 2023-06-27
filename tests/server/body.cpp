/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
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
#include <commonpp/core/string/stringify.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/http/Connection.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

#define BODY_SIZE 8192

static const std::string REQUEST =
// clang-format off
    "POST / HTTP/1.1\r\n"
    "User-Agent: curl/7.32.0\r\n"
    "Host: localhost:8000\r\n"
    "Accept: */*\r\n"
    "Content-Length: " BOOST_PP_STRINGIZE(BODY_SIZE) "\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "\r\n";
// clang-format on

std::string BODY;

static Connection* gconnection = nullptr;

void body_handler(const boost::system::error_code& ec, const char* buffer, size_t n)
{
    static std::string body_read;

    std::cout << "Received body part" << std::endl;

    if (buffer == nullptr)
    {
        std::cout << "Check" << std::endl;
        BOOST_CHECK_EQUAL(body_read, BODY);
        gconnection->response().setCode(HTTP::HttpCode::Ok).connectionShouldBeClosed(true);
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
    auto& request = connection->request();
    gconnection = connection;
    std::cout << "got a request" << std::endl;
    auto headers = request.getSortedHeaders();
    auto size = std::stoi(commonpp::string::stringify(headers["Content-Length"]));
    connection->read(size, &body_handler);
}

BOOST_AUTO_TEST_CASE(read_body)
{
    BODY.clear();
    for (int i = 0; i < BODY_SIZE; ++i)
    {
        BODY += char('a');
    }

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
    boost::asio::write(s, boost::asio::buffer(BODY));

    boost::asio::streambuf b;
    boost::asio::read_until(s, b, "\r\n");
    std::istream is(&b);
    std::string line;
    std::getline(is, line);
    boost::trim(line);
    std::cout << line << std::endl;

    BOOST_CHECK_EQUAL(line, "HTTP/1.1 200 Ok");
}

static void handler2(Connection* conn)
{
    read_whole_request(
        conn,
        [](std::unique_ptr<HTTP::helper::ReadWholeRequest> handler,
           const boost::system::error_code& ec)
        {
            if (ec)
            {
                Connection::releaseFromHandler(handler->connection);
                throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
            }
            else
            {
                std::cout << "Check" << std::endl;
                std::string str(handler->body.data(), handler->body.size());
                BOOST_CHECK_EQUAL(str, BODY);
                handler->connection->response().setCode(HTTP::HttpCode::Ok).connectionShouldBeClosed(true);
                handler->connection->sendResponse();
            }
        }
    );
}

BOOST_AUTO_TEST_CASE(read_everything1)
{
    BODY.clear();
    for (int i = 0; i < BODY_SIZE; ++i)
    {
        BODY += char('a');
    }

    HttpServer server;
    server.start();
    server.setSink(&handler2);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    boost::asio::write(s, boost::asio::buffer(REQUEST));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    boost::asio::write(s, boost::asio::buffer(BODY));

    boost::asio::streambuf b;
    boost::asio::read_until(s, b, "\r\n");
    std::istream is(&b);
    std::string line;
    std::getline(is, line);
    boost::trim(line);
    std::cout << line << std::endl;

    BOOST_CHECK_EQUAL(line, "HTTP/1.1 200 Ok");
}

static void handler3(Connection* conn)
{
    read_whole_request(
        conn,
        [](std::unique_ptr<HTTP::helper::ReadWholeRequest> handler,
           const boost::system::error_code& ec)
        {
            if (ec)
            {
                Connection::releaseFromHandler(handler->connection);
            }
            else
            {
                BOOST_REQUIRE(false);
            }
        }
    );
}

BOOST_AUTO_TEST_CASE(read_everything_with_err)
{
    BODY.clear();
    for (int i = 0; i < BODY_SIZE / 2; ++i)
    {
        BODY += char('a');
    }

    HttpServer server;
    server.start();
    server.setSink(&handler3);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    boost::asio::write(s, boost::asio::buffer(REQUEST));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    boost::asio::write(s, boost::asio::buffer(BODY));
    s.close();
}

static void handler4(Connection* conn)
{
    read_whole_request(
        conn,
        [](std::unique_ptr<HTTP::helper::ReadWholeRequest> handler,
           const boost::system::error_code& ec)
        {
            if (ec == boost::asio::error::message_size)
            {
                handler->connection->response().connectionShouldBeClosed(true);
                handler->connection->sendResponse();
                return;
            }
            else if (ec)
            {
                Connection::releaseFromHandler(handler->connection);
                throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
            }
            else
            {
                BOOST_REQUIRE(false);
                Connection::releaseFromHandler(handler->connection);
            }
        },
        10
    );
}

BOOST_AUTO_TEST_CASE(body_too_big)
{
    BODY.clear();
    for (int i = 0; i < BODY_SIZE; ++i)
    {
        BODY += char('a');
    }

    HttpServer server;
    server.start();
    server.setSink(&handler4);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    boost::asio::write(s, boost::asio::buffer(REQUEST));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    boost::asio::write(s, boost::asio::buffer(BODY));

    boost::asio::streambuf b;
    boost::asio::read_until(s, b, "\r\n");
    std::istream is(&b);
    std::string line;
    std::getline(is, line);
    boost::trim(line);
    std::cout << line << std::endl;

    BOOST_CHECK_EQUAL(line, "HTTP/1.1 413 RequestEntityTooLarge");
}
