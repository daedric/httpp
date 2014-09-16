/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
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

void chunked_data_handler(Connection* connection, Request&& )
{
    auto streaming = []() -> std::string 
    {
        
        return std::string("BLAH!");
    };

	connection->response()
		.setCode(HTTP::HttpCode::Ok)
		.setBody(streaming)
		.connectionShouldBeClosed(true);
	connection->sendResponse();
}


BOOST_AUTO_TEST_CASE(test_transfer_encoding_header_is_set_correctly)
{
    HttpServer server;
    server.start();
    server.setSink(&chunked_data_handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");

    auto response = client.get(std::move(request));
	auto headers = response.getSortedHeaders();
	BOOST_CHECK_EQUAL(headers["Transfer-Encoding"], "chunked");
}
