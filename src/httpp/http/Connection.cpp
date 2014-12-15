/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Connection.hpp"

#include <sstream>

#include <boost/log/trivial.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/ThreadPool.hpp"
#include "httpp/utils/VectorStreamBuf.hpp"

namespace HTTPP
{
namespace HTTP
{
const size_t Connection::BUF_SIZE = 8196;


Connection::Connection(HTTPP::HttpServer& handler,
                       boost::asio::io_service& service,
                       UTILS::ThreadPool& pool,
                       boost::asio::ssl::context* ctx
                       )
: handler_(handler)
, pool_(pool)
, socket_(service)
{
    if (ctx)
    {
        ssl_socket_.reset(new SSLSocket(socket_, *ctx));
    }
}

Connection::~Connection()
{
    BOOST_LOG_TRIVIAL(debug) << "Disconnect client";
    cancel();
    close();

    if (is_owned_)
    {
        BOOST_LOG_TRIVIAL(error) << "A connection is destroyed manually, this "
                                    "should always be done by the HttpServer";
        handler_.destroy(this, false);
    }
}

template <typename Buffer, typename Handler>
void Connection::async_read_some(Buffer&& buffer, Handler&& handler)
{
    if (ssl_socket_)
    {
        return ssl_socket_->async_read_some(std::forward<Buffer>(buffer),
                                            std::forward<Handler>(handler));
    }
    else
    {
        return socket_.async_read_some(std::forward<Buffer>(buffer),
                                       std::forward<Handler>(handler));
    }
}

void Connection::releaseFromHandler(Connection* connection)
{
    if (!connection->own())
    {
        throw std::logic_error("Invalid connection state");
    }

    release(connection);
}

void Connection::release(Connection* connection)
{
    connection->cancel();
    connection->close();

    delete connection;
}

void Connection::cancel() noexcept
{
    boost::system::error_code ec;
    socket_.cancel(ec);
}

void Connection::disown() noexcept
{
    bool expected = true;
    if (!is_owned_.compare_exchange_strong(expected, false))
    {
        BOOST_LOG_TRIVIAL(warning) << "Disown a connection already disowned";
    }
}

bool Connection::shouldBeDeleted() const noexcept
{
    return should_be_deleted_;
}

void Connection::markToBeDeleted() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!should_be_deleted_)
    {
        should_be_deleted_ = true;
        BOOST_LOG_TRIVIAL(debug) << "Connection marked to be deleted: " << this;
        cancel();
        close();
    }
}

void Connection::close() noexcept
{
    boost::system::error_code ec;
    if (ssl_socket_)
    {
        ssl_socket_->shutdown(ec);
    }
    else
    {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    }

    socket_.close(ec);
}

std::string Connection::source() const
{
    boost::system::error_code ec;
    const auto& remote_endpoint = socket_.remote_endpoint(ec);

    if (ec)
    {
        handler_.connection_error(const_cast<Connection*>(this), ec);
        return ec.message();
    }

    std::ostringstream source;
    source << remote_endpoint.address().to_string() << ':'
           << remote_endpoint.port();

    return source.str();
}

void Connection::start()
{
    size_ = buffer_.size();
    if (!size_)
    {
        buffer_.resize(BUF_SIZE);
    }
    response_ = Response();

    if (ssl_socket_ && need_handshake_)
    {
        need_handshake_ = false;
        ssl_socket_->async_handshake(boost::asio::ssl::stream_base::server,
                                     [this](boost::system::error_code const& ec)
                                     {
            if (ec)
            {
                handler_.connection_error(this, ec);
                return;
            }
            read_request();
        });
    }
    else
    {
        read_request();
    }
}

void Connection::read_request()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (shouldBeDeleted())
    {
        lock.unlock();
        handler_.destroy(this);
        return;
    }

    if (Parser::isComplete(buffer_.data(), size_))
    {
        UTILS::VectorStreamBuf buf(buffer_, size_);
        std::istream is(std::addressof(buf));
        Request request;
        if (Parser::parse(is, request))
        {
            BOOST_LOG_TRIVIAL(trace) << "Received a request from: " << source()
                                     << ": " << request;

            buf.shrinkVector();

            lock.unlock();
            handler_.connection_notify_request(this, std::move(request));
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning)
                << "Invalid request received from: " << source();
            BOOST_LOG_TRIVIAL(error) << std::string(buffer_.data(), size_);

            response_ = Response(
                    HttpCode::BadRequest,
                    std::string("An error occured in the request parsing indicating an error"));
            response_.connectionShouldBeClosed(true);

            lock.unlock();
            disown();
            sendResponse();
        }
    }
    else
    {
        if (size_ == buffer_.size())
        {
            buffer_.resize(buffer_.size() + BUF_SIZE);
        }

        char* data = buffer_.data();
        data += size_;

        async_read_some(boost::asio::buffer(data, buffer_.capacity() - size_),
                        [&](boost::system::error_code const& ec, size_t size)
                        {
            if (ec)
            {
                handler_.connection_error(this, ec);
                return;
            }

            this->size_ += size;
            read_request();
        });
    }
}

void Connection::sendResponse(Callback&& cb)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (shouldBeDeleted())
    {
        lock.unlock();
        handler_.destroy(this);
        return;
    }

    auto handler = [cb, this](boost::system::error_code const& ec, size_t)
    {
        if (ec)
        {
            handler_.connection_error(this, ec);
            return;
        }
        cb();
    };

    if (ssl_socket_)
    {
        response_.sendResponse(*ssl_socket_, std::move(handler));
    }
    else
    {
        response_.sendResponse(socket_, std::move(handler));
    }
}

bool Connection::own() noexcept
{
    bool expected = false;
    return is_owned_.compare_exchange_strong(expected, true);
}

void Connection::sendResponse()
{
    if (!own())
    {
        BOOST_LOG_TRIVIAL(error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    sendResponse([this] { recycle(); });
}

void Connection::sendContinue(Callback&& cb)
{
    if (!own())
    {
        BOOST_LOG_TRIVIAL(error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    response_
        .setBody("")
        .setCode(HttpCode::Continue);

    sendResponse([this, cb]
                 {
                     disown();
                     cb();
                 });
}

void Connection::recycle()
{
    if (shouldBeDeleted() || response_.connectionShouldBeClosed())
    {
        handler_.destroy(this);
        return;
    }
    else
    {
        if (response_.isComplete())
        {
            handler_.connection_recycle(this);
        }
    }
}

} // namespace HTTP
} // namespace HTTPP

