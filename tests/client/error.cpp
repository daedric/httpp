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

#include <functional>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;
using HTTPP::UTILS::RequestError;

void handler(Connection* connection)
{
    Connection::releaseFromHandler(connection);
}

BOOST_AUTO_TEST_CASE(wild_disconnection)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");
    BOOST_CHECK_THROW(client.get(std::move(request)), RequestError);
}

BOOST_AUTO_TEST_CASE(no_server)
{
    HttpClient client;

    HttpClient::Request request;
    request.url("http://localhost:8080");
    BOOST_CHECK_THROW(client.get(std::move(request)), RequestError);

    try
    {
        request.url("http://localhost:8080");
        client.get(std::move(request));
    }
    catch (RequestError& ex)
    {
        BOOST_CHECK_NO_THROW(std::rethrow_if_nested(ex));

        auto request = ex.moveRequest();
        (void)request;
    }
}

BOOST_AUTO_TEST_CASE(bad_server)
{
    commonpp::thread::ThreadPool pool(1);
    pool.start();
    boost::asio::io_service& service = pool.getService();

    boost::asio::ip::tcp::acceptor acceptor(service);
    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("0.0.0.0"), 8080);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(boost::asio::socket_base::max_connections);

    boost::asio::ip::tcp::socket socket(service);

    acceptor.async_accept(socket, [&](const boost::system::error_code& error)
            {
                GLOG(debug) << "Connection received";
                if (error)
                {
                    throw UTILS::convert_boost_ec_to_std_ec(error);
                }
                boost::asio::streambuf buffer;
                boost::asio::read_until(socket, buffer, "\r\n\r\n");
                std::istream is(&buffer);
                while (is)
                {
                    std::string str;
                    is >> str;
                    GLOG(debug) << "receive: " << str;
                }

                GLOG(debug) << "Send payload";

                boost::asio::write(
                    socket,
                    boost::asio::buffer("HTTP/1.1 200 OK\r\n"
                                        "Content-Length: 0\r\n"
                                        "Connection: Close\r\n"
                                        "This is an invalid header\r\n"
                                        "\r\n"));
                GLOG(debug) << "Payload sent";
            });

    HttpClient client;
    HttpClient::Request request;
    request.url("http://localhost:8080");
    try
    {
        client.get(std::move(request));
    }
    catch (const RequestError& ex)
    {
        BOOST_CHECK_THROW(std::rethrow_if_nested(ex), std::runtime_error);
    }
    catch(...)
    {
        BOOST_CHECK(false);
    }

    pool.stop();
}


static Connection* conn_manager_deletion = nullptr;

void handler_manager_deletion(Connection* connection)
{
  conn_manager_deletion = connection;
}


BOOST_AUTO_TEST_CASE(manager_deletion)
{
    HttpServer server;
    server.start();
    server.setSink(&handler_manager_deletion);
    server.bind("localhost", "8080");

    bool notif = false;
    {
        HttpClient client;
        HttpClient::Request request;
        request.url("http://localhost:8080");
        client.async_get(std::move(request),
                         [&notif](HttpClient::Future future)
                         {
            notif = true;
            BOOST_CHECK_THROW(future.get(), HTTPP::UTILS::OperationAborted);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    BOOST_CHECK(notif);
    Connection::releaseFromHandler(conn_manager_deletion);
}
