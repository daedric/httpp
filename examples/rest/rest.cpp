/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2016 Thomas Sanchez.  All rights reserved.
 *
 */

#include <chrono>
#include <iostream>
#include <string>

#include <httpp/HttpServer.hpp>
#include <httpp/http/Connection.hpp>
#include <httpp/http/RestDispatcher.hpp>
#include <httpp/http/Utils.hpp>
#include <httpp/utils/Exception.hpp>

using HTTPP::HttpServer;
using HTTPP::HTTP::Connection;
using HTTPP::HTTP::HttpCode;
using HTTPP::HTTP::Method;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::RestDispatcher;
using HTTPP::HTTP::helper::ReadWholeRequest;

void handler_with_body(ReadWholeRequest::Handle handle)
{
    auto connection = handle->connection;
    // do something with handle->body
    connection->response().setCode(HttpCode::Ok);
    HTTPP::HTTP::setShouldConnectionBeClosed(connection->request(), connection->response());
    connection->sendResponse(); // connection pointer may become invalid
}

void handler_without_body(Connection* connection)
{
    connection->response().setCode(HttpCode::Ok);
    HTTPP::HTTP::setShouldConnectionBeClosed(connection->request(), connection->response());
    connection->sendResponse(); // connection pointer may become invalid
}

int main(int, char**)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::warning);
    HttpServer server(1);
    server.start();

    RestDispatcher dispatcher(server);
    dispatcher.add<Method::POST>("/", &handler_with_body);
    dispatcher.add<Method::GET>("/", &handler_without_body);

    dispatcher.add<Method::PUT, Method::DELETE_, Method::HEAD, Method::CONNECT>(
        "/",
        [](Connection* connection)
        {
            connection->response().setCode(HttpCode::Forbidden);
            HTTPP::HTTP::setShouldConnectionBeClosed(
                connection->request(), connection->response()
            );
            connection->sendResponse(); // connection pointer may become invalid
        }
    );

    server.bind("0.0.0.0", "8080");
    while (true)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
