/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <chrono>

#include <httpp/HttpServer.hpp>
#include <httpp/http/Utils.hpp>
#include <httpp/utils/Exception.hpp>

using HTTPP::HttpServer;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Connection;
using HTTPP::HTTP::HttpCode;

void body_handler(const Request& request, Connection* connection,
                  const boost::system::error_code& ec,
                  const char* buffer, size_t n)
{
    std::cout << "Received body part" << std::endl;
    if (ec == boost::asio::error::eof)
    {
        connection->response()
            .setCode(HttpCode::Ok)
            .setBody("request received entirely");
        HTTPP::HTTP::setShouldConnectionBeClosed(request, connection->response());
        connection->sendResponse(); // connection pointer may become invalid

        auto end = Request::Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            end - request.received);
        std::cout << "Request handled in: " << elapsed.count() << "us"
                  << std::endl;
    }
    else if (ec)
    {
        throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
    }
    else
    {
        std::cout.write(buffer,n) << std::endl;
    }
}

void handler(Connection* connection, Request&& request)
{
    std::cout << "got a request" << std::endl;
    auto headers = request.getSortedHeaders();
    auto const& content_length = headers["Content-Length"];
    if (content_length.empty())
    {
        std::ostringstream out;
        out << request;
        connection->response()
            .setCode(HttpCode::Ok)
            .setBody("request received: " + out.str());
        HTTPP::HTTP::setShouldConnectionBeClosed(request, connection->response());
        connection->sendResponse(); // connection pointer may become invalid
        auto end = Request::Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            end - request.received);
        std::cout << "Request handled in: " << elapsed.count() << "us"
                  << std::endl;
    }
    else
    {
        auto size = std::stoi(content_length);
        connection->readBody(size,
                             std::bind(&body_handler,
                                     request,
                                     connection,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
    }
}

int main(int, char**)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");
    server.bind("localhost", "8081");
    server.bind("localhost", "8082");
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
