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
#include <memory>
#include <atomic>
#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"
#include "httpp/utils/Exception.hpp"
#include "httpp/http/Utils.hpp"
#include <boost/log/trivial.hpp>

using namespace HTTPP;

using HTTPP::HttpServer;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

const int DEFAULT_NUMBER_OF_CHUNKS = 3;
const int DEFAULT_CHUNK_SIZE = 100;

class TestChunkStreamer
{
public:
    TestChunkStreamer(int numberOfChunks, size_t sizeOfChunks)
    : m_numChunksRemaining(numberOfChunks), m_chunkSize(sizeOfChunks)
    {
    }

    std::string GenerateNextChunk()
    {
        if (m_numChunksRemaining > 0)
        {
            BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ":Sending Chunk";
            m_numChunksRemaining--;
            return std::string(m_chunkSize, 'X');
        }
        else
        {
            BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ":End of Stream";
            return "";
        }
    }

private:
    int m_numChunksRemaining;
    size_t m_chunkSize;
};

void chunked_data_handler(Connection* connection, Request&&)
{
    auto s = std::make_shared<TestChunkStreamer>(DEFAULT_NUMBER_OF_CHUNKS,
                                                 DEFAULT_CHUNK_SIZE);
    auto body = [s]()
    { return s->GenerateNextChunk(); };

    connection->response().setCode(HTTP::HttpCode::Ok).setBody(body);
    connection->sendResponse();
}

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
    std::string body(response.body.begin(), response.body.end());
    auto headers = response.getSortedHeaders();

    BOOST_CHECK_EQUAL(headers["Transfer-Encoding"], "chunked");
    BOOST_CHECK_EQUAL("", body);
    BOOST_CHECK_EQUAL(getDefaultMessage(HTTP::HttpCode::Ok),
                      getDefaultMessage(response.code));
}

//
// Test we can cancel a streaming request. Also check the client api gets the
// correct exception.
//
void infinite_response(Connection* connection, Request&&)
{
    connection->response().setCode(HTTP::HttpCode::Ok).setBody([]()
                                                               { return "XXX"; });
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
