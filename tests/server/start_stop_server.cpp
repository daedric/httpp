/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

static const std::string REQUEST = "GET / HTTP/1.1\r\n"
                                   "Host: localhost:8000\r\n"
                                   "\r\n";

static Connection* gconn = nullptr;
void handler(Connection* connection)
{
    std::cout << "Got a request";
    gconn = connection;
}

BOOST_AUTO_TEST_CASE(simple)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::trace);
    commonpp::core::enable_console_logging();

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

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        gconn->response().setBody("Helloworld");
        gconn->sendResponse();
    });

    server.stop();
}

static const std::string REQUEST_INCOMPLETE = "GET / HTTP/1.1";

BOOST_AUTO_TEST_CASE(during_read)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::trace);
    commonpp::core::enable_console_logging();

    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost");

    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "8000"}));
    boost::asio::write(s, boost::asio::buffer(REQUEST_INCOMPLETE));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    server.stop();
}
