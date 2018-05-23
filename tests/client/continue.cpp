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

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Connection;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;

void handler(Connection* connection)
{
    auto& request = connection->request();
    auto headers = request.getSortedHeaders();
    if (headers["Expect"] == "100-continue")
    {
        connection->sendContinue([connection] {
            read_whole_request(
                connection,
                [](std::unique_ptr<HTTPP::HTTP::helper::ReadWholeRequest> hndl,
                   const boost::system::error_code& ec) {
                    if (ec)
                    {
                        throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
                    }

                    hndl->connection->response()
                        .setCode(HTTP::HttpCode::Ok)
                        .setBody("Body received");
                    hndl->connection->sendResponse();
                });
        });
    }
}

BOOST_AUTO_TEST_CASE(expect_continue)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    using PostEncoding = HttpClient::Request::PostEncoding;

    HttpClient::Request request;
    request.url("http://localhost:8080")
        .pushPostData("Hello", "world")
        .pushPostData("test", "123!^&%@#^&%$");

    auto response = client.post(std::move(request));
    std::string body(response.body.data(), response.body.size());
    BOOST_CHECK_EQUAL(body, "Body received");
}
