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
#include <memory>
#include <atomic>
#include <string>
#include <functional>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"
#include "httpp/utils/Exception.hpp"
#include "httpp/http/Utils.hpp"

using namespace HTTPP;

using HTTPP::HttpServer;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

const int DEFAULT_NUMBER_OF_CHUNKS = 3;
const int DEFAULT_CHUNK_SIZE = 100;

//
// Helper that constructs a stream functor for the specified number of chunks,
// each chunk being 'chunkSize' bytes.
//
std::function<std::string()> make_stream(int numChunks, int chunkSize)
{
    auto stream = [numChunks,chunkSize]() mutable -> std::string
    {
        if (numChunks > 0)
        {
            GLOG(debug) << __FUNCTION__ << ":Sending Chunk ";
            numChunks--;
            return std::string(chunkSize, 'X');
        }
        else
        {
            GLOG(debug) << __FUNCTION__ << ":End of Stream ";
            return "";
        }
    };
    return stream;
}


void chunked_data_handler(Connection* connection, Request&&)
{    
    auto body = make_stream(DEFAULT_NUMBER_OF_CHUNKS, DEFAULT_CHUNK_SIZE);
    connection->response().setCode(HTTP::HttpCode::Ok).setBody(std::move(body));
    connection->sendResponse();
}

//
// Test we correctly set the Transfer-Encoding header when streaming 
//
BOOST_AUTO_TEST_CASE(test_transfer_encoding_header_is_set_correctly)
{
    HttpServer server;
    server.start();
    server.setSink(&chunked_data_handler);
    server.bind("localhost", "8080");

    HttpClient::Request request;
    request.url("http://localhost:8080");
    HttpClient client;

    auto response = client.get(std::move(request));
    auto headers = response.getSortedHeaders();
    BOOST_CHECK_EQUAL(headers["Transfer-Encoding"], "chunked");
}

//
// Test that the client will receive the expected payload. Since we use CUrl, we
// receive the
// re-assembled payload, not the raw payload of individual chunks.
//
BOOST_AUTO_TEST_CASE(test_client_receives_expected_body)
{
    HttpServer server;
    server.start();
    server.setSink(&chunked_data_handler);
    server.bind("localhost", "8080");

    HttpClient::Request request;
    request.url("http://localhost:8080");
    HttpClient client;

    auto response = client.get(std::move(request));
    std::string body(response.body.begin(), response.body.end());

    std::string expectedBody(DEFAULT_CHUNK_SIZE * DEFAULT_NUMBER_OF_CHUNKS, 'X');
    BOOST_CHECK_EQUAL(expectedBody, body);
}

//
// Test we can send an empty response with no data. Should return 200 OK, but
// nothing in the payload.
//
void empty_chunk_response(Connection* connection, Request&&)
{
    auto empty_response = []()
    { return ""; };

    connection->response().setCode(HTTP::HttpCode::Ok).setBody(empty_response);
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(test_empty_chunk_response)
{
    HttpServer server;
    server.start();
    server.setSink(&empty_chunk_response);
    server.bind("localhost", "8080");

    HttpClient::Request request;
    request.url("http://localhost:8080");
    HttpClient client;

    auto response = client.get(std::move(request));    
    auto headers = response.getSortedHeaders();

    BOOST_CHECK_EQUAL(headers["Transfer-Encoding"], "chunked");
    BOOST_CHECK_EQUAL(true, response.body.empty());
    BOOST_CHECK_EQUAL(getDefaultMessage(HTTP::HttpCode::Ok),
                      getDefaultMessage(response.code));
}

//
// Test we can cancel a streaming request. Also check the client api gets the
// correct exception.
//
void infinite_response(Connection* connection, Request&&)
{
    connection->response()
        .setCode(HTTP::HttpCode::Ok)
        .setBody([]() { return "XXX"; });
    connection->sendResponse();
}

BOOST_AUTO_TEST_CASE(test_cancelling_request_while_streaming_stops_functor)
{
    HttpServer server;
    server.start();
    server.setSink(&infinite_response);
    server.bind("localhost", "8080");
    BOOST_CHECK_EQUAL(server.getNbConnection(), 1);

    {
        HttpClient::Request request;
        request.url("http://localhost:8080");
        HttpClient client;

        auto handler = client.async_get(
            std::move(request),
            [](HttpClient::Future&& fut)
            { BOOST_CHECK_THROW(fut.get(), HTTPP::UTILS::OperationAborted); });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        BOOST_CHECK_EQUAL(server.getNbConnection(), 2);
        handler.cancelOperation();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(server.getNbConnection(), 1);
}



//
// Test the raw response contains the correct header and chunk data.
//

std::string GetRawResponse()
{
    using boost::asio::ip::tcp;

    boost::asio::io_service io_service;
    tcp::socket sock(io_service);
    tcp::resolver resolver(io_service);

    // send request
    const std::string REQUEST("GET / HTTP/1.1\r\n\r\n");
    boost::asio::connect(sock, resolver.resolve({ "localhost", "8080" }));
    boost::asio::write(sock, boost::asio::buffer(REQUEST));

    // read until end-of-stream
    boost::asio::streambuf response;
    auto bytes = boost::asio::read_until(sock, response, "\r\n0\r\n\r\n");

    // convert buffer to a string
    auto responseData = response.data();
    std::string str(boost::asio::buffers_begin(responseData), boost::asio::buffers_begin(responseData) + bytes);
    return str;
}


BOOST_AUTO_TEST_CASE(test_format_of_raw_response)
{
    HttpServer server;
    server.start();
    server.setSink(&chunked_data_handler);
    server.bind("localhost", "8080");

    auto rawResponse = GetRawResponse();
    auto startOfBody = rawResponse.find("\r\n\r\n");
    BOOST_CHECK(startOfBody != std::string::npos);
    startOfBody += 4;

    // each chunk should start with "64\r\n" and end with "\r\n".
    auto chunkIterator = startOfBody;
    for (int i=0; i<3; i++)
    {
        auto chunkHeader = rawResponse.substr(chunkIterator, 4);
        BOOST_CHECK_EQUAL(chunkHeader, "64\r\n");
        chunkIterator += 4;
        auto chunkBody = rawResponse.substr(chunkIterator, 100);        
        BOOST_CHECK_EQUAL(chunkBody, std::string(DEFAULT_CHUNK_SIZE, 'X'));   
        chunkIterator += 100;
        auto chunkTrailer = rawResponse.substr(chunkIterator, 2);
        BOOST_CHECK_EQUAL(chunkTrailer, "\r\n");
        chunkIterator += 2;
    }
}

