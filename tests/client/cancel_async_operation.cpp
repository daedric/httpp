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
#include <iostream>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;


static Connection* gconn = nullptr;
void handler(Connection* connection)
{
    gconn = connection;
}

BOOST_AUTO_TEST_CASE(cancel_async_operation)
{
    HttpClient client;

    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .joinUrlPath("test")
        .joinUrlPath("kiki", true);

    auto handler = client.async_get(
            std::move(request),
            [](HttpClient::Future&& fut)
            { BOOST_CHECK_THROW(fut.get(), HTTPP::UTILS::OperationAborted); });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    handler.cancelOperation();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    Connection::releaseFromHandler(gconn);
    server.stop();
    GLOG(error) << "operation cancelled";
}

static std::atomic_int nb_gconns = { 0 };
static std::vector<Connection*> gconns;
void handler_push(Connection* connection)
{
    ++nb_gconns;
    gconns.push_back(connection);
}


BOOST_AUTO_TEST_CASE(delete_pending_connection)
{
    HttpServer server;
    server.start();
    server.setSink(&handler_push);
    server.bind("localhost", "8080");

    static const auto NB_CONN = 100;
    std::atomic_int nb_cb {0};

    {
        HttpClient client;

        HttpClient::Request request;
        request.url("http://localhost:8080");

        for (int i = 0; i < NB_CONN; ++i)
        {
            client.async_get(HttpClient::Request{ request },
                             [&nb_cb](HttpClient::Future&& fut)
                             {
                ++nb_cb;
                BOOST_CHECK_THROW(fut.get(), HTTPP::UTILS::OperationAborted);
            });
        }

        while (nb_gconns != NB_CONN)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    server.stopListeners();
    std::for_each(
        std::begin(gconns), std::end(gconns), &Connection::releaseFromHandler);
    server.stop();

    BOOST_CHECK_EQUAL(nb_cb.load(), NB_CONN);
}

BOOST_AUTO_TEST_CASE(delete_pending_connection_google)
{
    HttpClient client;

    HttpClient::Request request;
    request.url("http://google.com").followRedirect(true);

    for (int i = 0; i < 10; ++i)
    {
        client.async_get(HttpClient::Request{ request },
                         [](HttpClient::Future&& fut)
                         {
            BOOST_CHECK_THROW(fut.get(), HTTPP::UTILS::OperationAborted);
        });
    }
}

void handler2(Connection* c)
{
    c->response().setCode(HTTPP::HTTP::HttpCode::Ok);
    c->sendResponse();
}

BOOST_AUTO_TEST_CASE(late_cancel)
{
    HttpServer server;
    server.start();
    server.setSink(&handler2);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");

    bool ok = false;
    auto handler = client.async_get(std::move(request),
                                    [&ok](HttpClient::Future&& fut)
                                    {
        ok = true;
        GLOG(debug) << "Response received";
        fut.get();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK(ok);
    handler.cancelOperation();
    GLOG(error) << "operation cancelled";
}

