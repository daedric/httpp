/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
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

void chunked_handler(Connection* connection, Request&& request)
{
    auto numChunks = 10;
    auto chunkSize = 8192;

    auto body = [numChunks, chunkSize]() mutable -> std::string
    {
        if (numChunks-- > 0)
        {
            return std::string(chunkSize, 'X');
        }
        else
        {
            return std::string("");
        }
    };

    connection->response().setCode(HttpCode::Ok).setBody(body);
    HTTPP::HTTP::setShouldConnectionBeClosed(request, connection->response());
    connection->sendResponse();
}

int main(int, char**)
{
    HttpServer server;
    server.start();
    server.setSink(&chunked_handler);
    server.bind("localhost", "8080");
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        
    }
}
