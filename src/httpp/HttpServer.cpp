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

#include "httpp/utils/Exception.hpp"
#include "httpp/http/Utils.hpp"

namespace HTTPP
{
struct HttpServer::Acceptor : boost::asio::ip::tcp::acceptor
{
    Acceptor(boost::asio::io_service& service)
    : boost::asio::ip::tcp::acceptor(service)
    {
    }

    void setSSLContext(SSLContext ctx)
    {
        ssl_ctx.reset(
            new boost::asio::ssl::context(boost::asio::ssl::context::sslv23));
        if (!((ctx.cert_file.empty() ^ ctx.cert_buffer.empty()) &&
              (ctx.key_file.empty() ^ ctx.key_buffer.empty())))
        {
            throw std::invalid_argument(
                "Cert and Key file/buffer are required");
        }

        auto flags = boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 |
                     boost::asio::ssl::context::no_sslv3 |
                     boost::asio::ssl::context::no_tlsv1 |
                     boost::asio::ssl::context::single_dh_use;

        ssl_ctx->set_options(flags);

        if (!ctx.cert_file.empty())
        {
            ssl_ctx->use_certificate_file(ctx.cert_file,
                                          boost::asio::ssl::context::pem);
        }
        else
        {
            ssl_ctx->use_certificate(boost::asio::buffer(ctx.cert_buffer),
                                     boost::asio::ssl::context::pem);
        }

        if (!ctx.key_file.empty())
        {
            ssl_ctx->use_private_key_file(ctx.key_file,
                                          boost::asio::ssl::context::pem);
        }
        else
        {
            ssl_ctx->use_private_key(boost::asio::buffer(ctx.key_buffer),
                                     boost::asio::ssl::context::pem);
        }

        if (!ctx.dh_file.empty())
        {
            ssl_ctx->use_tmp_dh_file(ctx.dh_file);
        }
        else if (!ctx.dh_buffer.empty())
        {
            ssl_ctx->use_tmp_dh(boost::asio::buffer(ctx.dh_buffer));
        }
    }

    std::unique_ptr<boost::asio::ssl::context> ssl_ctx;
};

HttpServer::HttpServer(size_t threads)
: pool_(std::make_shared<UTILS::ThreadPool>(threads))
{
}

static void empty_deleter(UTILS::ThreadPool*)
{
}

HttpServer::HttpServer(UTILS::ThreadPool& pool)
: pool_(std::addressof(pool), &empty_deleter)
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
    pool_->start(fct);
}

void HttpServer::stopListeners()
{
    for (auto acc : acceptors_)
    {
        boost::system::error_code ec;
        acc->cancel(ec);
        acc->close(ec);
    }

    acceptors_.clear();

    while (running_acceptors_)
    {
        std::this_thread::yield();
    }

}

void HttpServer::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;

    stopListeners();

    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto connection : connections_)
        {
            connection->markToBeDeleted();
        }
    }

    while (connection_count_)
    {
        std::this_thread::yield();
    }

    pool_->stop();
}

void HttpServer::bind(const std::string& address, const std::string& port)
{
    auto acc = HttpServer::bind(pool_->getService(), address, port);
    BOOST_LOG_TRIVIAL(debug) << "Bind address: " << address
                             << " on port: " << port;
    acceptors_.push_back(acc);
    ++running_acceptors_;
    start_accept(acc);
}

void HttpServer::bind(const std::string& address,
                      SSLContext ctx,
                      const std::string& port)
{

    auto acc = HttpServer::bind(pool_->getService(), address, port);
    acc->setSSLContext(std::move(ctx));
    BOOST_LOG_TRIVIAL(debug) << "SSL bind address: " << address
                             << " on port: " << port;
    acceptors_.push_back(acc);
    ++running_acceptors_;
    start_accept(acc);
}

void HttpServer::mark(ConnectionPtr connection)
{
    ++connection_count_;
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.emplace_back(connection);
}

void HttpServer::destroy(ConnectionPtr connection, bool release)
{
    connection->markToBeDeleted();

    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = std::find_if(std::begin(connections_),
                               std::end(connections_),
                               [&](ConnectionPtr conn)
                               { return connection == conn; });
        if (it == std::end(connections_))
        {
            return;
        }
        else
        {
            --connection_count_;
            connections_.erase(it);
        }
    }

    if (release)
    {
        connection->disown();
        HTTP::Connection::release(connection);
    }
}

void HttpServer::start_accept(AcceptorPtr acceptor)
{
    if (running_)
    {

        auto connection = new HTTP::Connection(*this, pool_->getService(),
                                               *pool_, acceptor->ssl_ctx.get());
        mark(connection);

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
        destroy(connection);

        if (error == boost::asio::error::operation_aborted)
        {
            BOOST_LOG_TRIVIAL(info) << "Cancel listener";
        }
        else
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error during accept: " << error.message();
        }

        --running_acceptors_;
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
            destroy(connection);
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
            throw UTILS::convert_boost_ec_to_std_ec(error);
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
        throw UTILS::convert_boost_ec_to_std_ec(error);
    }

    acceptor->listen(boost::asio::socket_base::max_connections, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "Cannot listen " << host << " on " << port
                                 << ", error msg: " << error.message();
        throw UTILS::convert_boost_ec_to_std_ec(error);
    }
    return acceptor;
}

void HttpServer::connection_error(ConnectionPtr connection,
                                  const boost::system::error_code& ec)
{
    if (ec != boost::asio::error::eof &&
        ec != boost::asio::error::connection_reset &&
        ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Got an error from a Connection: " << ec.message();
    }

    destroy(connection);
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
        connection->disown();
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
