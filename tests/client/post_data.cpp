/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

void handler(Connection* connection)
{
    read_everything(connection, [](std::unique_ptr<HTTP::helper::ReadEverything> hndl,
                                   const boost::system::error_code& ec) {
        if (ec)
        {
            throw UTILS::convert_boost_ec_to_std_ec(ec);
        }

        hndl->connection->response()
            .setCode(HTTP::HttpCode::Ok)
            .connectionShouldBeClosed(true)
            .setBody("");
        hndl->connection->sendResponse();
    });
}

BOOST_AUTO_TEST_CASE(post_data)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    using PostEncoding = HttpClient::Request::PostEncoding;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .pushPostData("Hello", "world", PostEncoding::FormUrlEncoded)
        .pushPostData("test", "123!^&%@#^&%$", PostEncoding::FormUrlEncoded);

    client.post(std::move(request));
}
