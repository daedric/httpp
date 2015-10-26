/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Connection.hpp"

#include <sstream>

#include "httpp/detail/config.hpp"
#include "httpp/HttpServer.hpp"
#include "httpp/http/Parser.hpp"
#include "httpp/utils/VectorStreamBuf.hpp"

namespace HTTPP
{
namespace HTTP
{
const size_t Connection::BUF_SIZE = 8192;


Connection::Connection(HTTPP::HttpServer& handler,
                       boost::asio::io_service& service,
                       boost::asio::ssl::context* ctx
                       )
: handler_(handler)
, socket_(service)
{
    if (ctx)
    {
        ssl_socket_.reset(new SSLSocket(socket_, *ctx));
    }
}

Connection::~Connection()
{
    LOG(logger_, debug) << "Disconnect client";
    cancel();
    close();

    if (is_owned_)
    {
        LOG(logger_, error) << "A connection is destroyed manually, this "
                               "should always be done by the HttpServer";
        handler_.destroy(this, false);
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

void Connection::disown()
{
    bool expected = true;
    if (!is_owned_.compare_exchange_strong(expected, false))
    {
        LOG(logger_, warning) << "Disown a connection already disowned";
    }
}

bool Connection::shouldBeDeleted() const noexcept
{
    return should_be_deleted_;
}

void Connection::markToBeDeleted()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!should_be_deleted_)
    {
        should_be_deleted_ = true;
        LOG(logger_, debug) << "Connection marked to be deleted: " << this;
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
    if (!own())
    {
        throw std::logic_error("Invalid connection state");
    }

    // Maybe we have the beginning of the next request in the
    // body_buffer_
    if (not body_buffer_.empty())
    {
        request_buffer_.swap(body_buffer_);
        size_ = request_buffer_.size();
    }
    else
    {
        size_ = 0;
    }

    request_buffer_.resize(BUF_SIZE, 0);

    body_buffer_.clear();
    body_buffer_.reserve(BUF_SIZE);

    request_.clear();
    response_.clear();

    if (ssl_socket_ && need_handshake_)
    {
        need_handshake_ = false;
        ssl_socket_->async_handshake(boost::asio::ssl::stream_base::server,
                                     [this](boost::system::error_code const& ec) {
                                         if (ec)
                                         {
                                             disown();
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
    if (shouldBeDeleted())
    {
        disown();
        handler_.destroy(this);
        return;
    }

    if (Parser::isComplete(request_buffer_.data(), size_))
    {
        request_.setDate();
#if PARSER_BACKEND == STREAM_BACKEND
        UTILS::VectorStreamBuf buf(request_buffer_, size_);
        std::istream is(std::addressof(buf));
        if (Parser::parse(is, request_))
        {
            DLOG(logger_, trace) << "Received a request from: " << source()
                                 << ": " << request_;

            buf.shrinkVector();
            body_buffer_.swap(request_buffer_);

            disown();
            handler_.connection_notify_request(this);
        }
#elif PARSER_BACKEND == RAGEL_BACKEND
        const char* begin = request_buffer_.data();
        const char* end = begin + size_;
        size_t consumed = 0;
        if (Parser::parse(begin, end, consumed, request_))
        {
            DLOG(logger_, trace) << "Received a request from: " << source()
                                 << ": " << request_;

            if (consumed != size_)
            {
                body_buffer_.insert(body_buffer_.begin(),
                                    request_buffer_.begin() + consumed,
                                    request_buffer_.begin() + size_);
                request_buffer_.resize(consumed);
            }

            disown();
            handler_.connection_notify_request(this);
        }
#endif
        else
        {
            LOG(logger_, warning)
                << "Invalid request received from: " << source() << "\n"
                << std::string(request_buffer_.data(), size_);

            response_ = Response(
                    HttpCode::BadRequest,
                    std::string("An error occured in the request parsing indicating an error"));
            response_.connectionShouldBeClosed(true);

            disown();
            sendResponse();
        }
    }
    else
    {
        if (size_ == request_buffer_.size())
        {
            request_buffer_.resize(request_buffer_.size() + BUF_SIZE);
        }

        char* data = request_buffer_.data();
        data += size_;

        async_read_some(
            boost::asio::buffer(data, request_buffer_.capacity() - size_),
            [this](boost::system::error_code const& ec, size_t size) {
                if (ec)
                {
                    disown();
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
    if (shouldBeDeleted())
    {
        disown();
        handler_.destroy(this);
        return;
    }

    auto handler = [cb, this](boost::system::error_code const& ec, size_t)
    {
        disown();
        if (ec)
        {
            handler_.connection_error(this, ec);
            return;
        }
        cb();
    };

    std::unique_lock<std::mutex> lock(mutex_);
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
        LOG(logger_, error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    sendResponse([this] { recycle(); });
}

void Connection::sendContinue(Callback&& cb)
{
    if (!own())
    {
        LOG(logger_, error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    response_
        .setBody("")
        .setCode(HttpCode::Continue);

    sendResponse([this, cb] { cb(); });
}

void Connection::recycle()
{
    // Here Connection is not owned.
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

