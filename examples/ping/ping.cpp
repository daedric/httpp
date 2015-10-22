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
#include <string>
#include <chrono>

#include <httpp/HttpServer.hpp>
#include <httpp/http/Utils.hpp>
#include <httpp/utils/Exception.hpp>

using HTTPP::HttpServer;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Connection;
using HTTPP::HTTP::HttpCode;

void handler(Connection* connection, Request&& request)
{
    connection->response().setCode(HttpCode::Ok);
    HTTPP::HTTP::setShouldConnectionBeClosed(request, connection->response());
    connection->sendResponse(); // connection pointer may become invalid
}

int main(int, char**)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::warning);
    HttpServer server(1);
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
