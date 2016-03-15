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
using HTTPP::HTTP::helper::ReadWholeRequest;
using HTTPP::HTTP::HttpCode;

void handler(Connection* connection)
{
    read_whole_request(connection, [](std::unique_ptr<ReadWholeRequest> handle,
                                      const boost::system::error_code& ec) {

        if (ec)
        {
            throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
        }

        auto connection = handle->connection;
        connection->response().setCode(HttpCode::Ok);
        HTTPP::HTTP::setShouldConnectionBeClosed(connection->request(),
                                                 connection->response());
        connection->sendResponse(); // connection pointer may become invalid
    });
}

int main(int ac, char** av)
{
    std::string port = "8080";

    if (ac > 1)
    {
        port = av[1];
    }

    {
        auto port_env = getenv("PORT");
        if (port_env)
        {
            port = port_env;
        }
    }

    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::warning);
    HttpServer server(1);
    server.start();
    server.setSink(&handler);
    server.bind("0.0.0.0", port);
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
