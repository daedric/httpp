/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/HttpServer.hpp"

#include <boost/log/trivial.hpp>
#include "httpp/http/Utils.hpp"

namespace HTTPP
{

HttpServer::HttpServer(size_t threads)
: service_(threads)
, pool_(threads, service_)
{
}

HttpServer::~HttpServer()
{
    if (running_)
    {
        stop();
    }
}

void HttpServer::start(ThreadInit fct)
{
    if (running_)
    {
        return;
    }
    running_ = true;
    pool_.start(fct);
}

void HttpServer::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;
    for (auto acc : acceptors_)
    {
        boost::system::error_code ec;
        acc->cancel(ec);
        acc->close(ec);
    }

    acceptors_.clear();
    pool_.stop();
}

void HttpServer::bind(const std::string& address, const std::string& port)
{
    auto acc = HttpServer::bind(service_, address, port);
    BOOST_LOG_TRIVIAL(debug) << "Bind address: " << address
                             << " on port: " << port;
    acceptors_.push_back(acc);
    start_accept(acc);
}

void HttpServer::start_accept(AcceptorPtr acceptor)
{
    if (running_)
    {
        auto connection = new HTTP::Connection(*this, service_, pool_);
        //std::make_shared<HTTP::Connection>(*this, service_, pool_);

        acceptor->async_accept(connection->socket_,
                               std::bind(&HttpServer::accept_callback,
                                         this,
                                         std::placeholders::_1,
                                         acceptor,
                                         connection));
    }
}

void HttpServer::accept_callback(const boost::system::error_code& error,
                                 AcceptorPtr acceptor,
                                 ConnectionPtr connection)
{

    if (error)
    {
        HTTP::Connection::disconnect(connection);

        if (error == boost::asio::error::operation_aborted)
        {
            BOOST_LOG_TRIVIAL(info) << "Cancel listener";
        }
        else
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error during accept: " << error.message();
        }

        return;
    }
    else
    {
        if (running_)
        {
            BOOST_LOG_TRIVIAL(debug)
                << "New connection accepted from: " << connection->source();
            connection->start();
        }
        else
        {
            HTTP::Connection::disconnect(connection);
        }
    }

    start_accept(acceptor);
}

HttpServer::AcceptorPtr HttpServer::bind(boost::asio::io_service& service,
                                         const std::string& host,
                                         const std::string& port)
{
    auto acceptor = AcceptorPtr(new Acceptor(service));
    boost::system::error_code error;
    auto addr = boost::asio::ip::address::from_string(host, error);
    boost::asio::ip::tcp::endpoint endpoint;
    if (error)
    {
        boost::asio::ip::tcp::resolver resolver(service);
        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query),
                                                 end;

        if (it == end)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error bad address (ip address in dotted format or ipv6 hex format: "
                << host << ", error msg: " << error.message();
            throw error;
        }
        else
        {
            endpoint = *it;
        }
    }
    else
    {
        endpoint = boost::asio::ip::tcp::endpoint(addr, std::stoi(port));
    }

    acceptor->open(endpoint.protocol());
    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), error);
    if (error)
    {

        BOOST_LOG_TRIVIAL(warning) << "Cannot set REUSEADDR on " << host
                                   << " on " << port
                                   << ", error msg: " << error.message();
    }

    acceptor->bind(endpoint, error);
    if (error)
    {

        BOOST_LOG_TRIVIAL(error) << "Cannot bind " << host << " on " << port
                                 << " error msg: " << error.message();
        throw error;
    }

    acceptor->listen(boost::asio::socket_base::max_connections, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "Cannot listen " << host << " on " << port
                                 << ", error msg: " << error.message();
        throw error;
    }
    return acceptor;
}

void HttpServer::connection_error(ConnectionPtr connection,
                                  const boost::system::error_code& ec)
{
    if (ec != boost::asio::error::eof)
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Got an error from a Connection: " << ec.message();
    }

    pool_.post([connection]
               { HTTP::Connection::disconnect(connection); });
}

void HttpServer::connection_recycle(ConnectionPtr connection)
{
    connection->start();
}

void HttpServer::connection_notify_request(ConnectionPtr connection,
                                           HTTP::Request&& request)
{
    if (sink_)
    {
        sink_(connection, std::forward<HTTP::Request>(request));
    }
    else
    {
        connection->response().setCode(HTTP::HttpCode::NoContent).setBody("");
        HTTP::setShouldConnectionBeClosed(request, connection->response());
        connection->sendResponse();
    }
}

}
