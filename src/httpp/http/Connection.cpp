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

#include "httpp/HttpServer.hpp"
#include "httpp/detail/config.hpp"
#include "httpp/http/Parser.hpp"
#include "httpp/utils/VectorStreamBuf.hpp"

namespace HTTPP
{
namespace HTTP
{

namespace connection_detail
{
DECLARE_LOGGER(conn_logger_, "httpp::HttpServer::Connection");
} // namespace connection_detail

using namespace connection_detail;

Connection::Connection(
    HTTPP::HttpServer& handler, boost::asio::io_service& service, boost::asio::ssl::context* ctx
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
    LOG(conn_logger_, debug) << "Disconnect client";
    cancel();
    close();

    if (is_owned_)
    {
        LOG(conn_logger_, error) << "A connection is destroyed manually, this "
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
        LOG(conn_logger_, warning) << "Disown a connection already disowned";
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
        LOG(conn_logger_, debug) << "Connection marked to be deleted: " << this;
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
        return ec.message();
    }

    std::ostringstream source;
    source << remote_endpoint.address().to_string() << ':' << remote_endpoint.port();

    return source.str();
}

void Connection::start()
{
    if (!own())
    {
        throw std::logic_error("Invalid connection state");
    }

    // Maybe we have the beginning of the next request in the
    // request_buffer_
    if (!request_buffer_.empty())
    {
        request_buffer_.erase(request_buffer_.begin(), request_buffer_.begin() + offset_body_end_);
        size_ = request_buffer_.size();
    }
    request_.clear();
    response_.clear();

    if (ssl_socket_ && need_handshake_)
    {
        need_handshake_ = false;
        ssl_socket_->async_handshake(
            boost::asio::ssl::stream_base::server,
            [this](const boost::system::error_code& ec)
            {
                if (ec)
                {
                    disown();
                    handler_.connection_error(this, ec);
                    return;
                }
                read_request();
            }
        );
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

    if (Parser::isComplete(request_buffer_.data(), request_buffer_.size()))
    {
        request_.setDate();
#if HTTPP_PARSER_BACKEND_IS_STREAM
        UTILS::VectorStreamBuf buf(request_buffer_, size_);
        std::istream is(std::addressof(buf));
        if (Parser::parse(is, request_))
        {
            DLOG(conn_logger_, trace)
                << "Received a request from: " << source() << ": " << request_;

            buf.shrinkVector();
            body_buffer_.swap(request_buffer_);

            disown();
            handler_.connection_notify_request(this);
        }
#elif HTTPP_PARSER_BACKEND_IS_RAGEL
        const char* begin = request_buffer_.data();
        const char* end = begin + size_;
        size_t consumed = 0;
        if (Parser::parse(begin, end, consumed, request_))
        {
            DLOG(conn_logger_, trace)
                << "Received a request from: " << source() << ": " << request_;

            offset_body_end_ = offset_body_start_ = consumed;
            disown();
            handler_.connection_notify_request(this);
        }
#endif
        else
        {
            LOG(conn_logger_, warning)
                << "Invalid request received from: " << source() << "\n"
                << std::string(request_buffer_.data(), size_);

            response_.setCode(HttpCode::BadRequest)
                .setBody(
                    "An error occured in the request parsing indicating an "
                    "error"
                )
                .connectionShouldBeClosed(true);
            disown();
            sendResponse();
        }
    }
    else
    {
        request_buffer_.resize(request_buffer_.capacity());
        if (size_ == request_buffer_.size())
        {
            request_buffer_.resize(request_buffer_.size() + BUF_SIZE);
        }

        char* data = request_buffer_.data();
        data += size_;

        async_read_some(
            boost::asio::buffer(data, BUF_SIZE),
            [this](const boost::system::error_code& ec, size_t size)
            {
                if (ec)
                {
                    disown();
                    handler_.connection_error(this, ec);
                    return;
                }

                size_ += size;
                request_buffer_.resize(size_);
                read_request();
            }
        );
    }
}

void Connection::reparse()
{
    request_.clear();
    const char* begin = request_buffer_.data();
    const char* end = begin + offset_body_start_;
    size_t consumed = 0;
    Parser::parse(begin, end, consumed, request_);
}

void Connection::sendResponse(Callback&& cb)
{
    if (shouldBeDeleted())
    {
        disown();
        handler_.destroy(this);
        return;
    }

    if (handler_.ev_hndl_)
    {
        handler_.ev_hndl_->response_send(this);
    }

    auto handler = [cb, this](const boost::system::error_code& ec, size_t)
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
        LOG(conn_logger_, error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    sendResponse(
        [this]
        {
            recycle();
        }
    );
}

void Connection::sendContinue(Callback&& cb)
{
    if (!own())
    {
        LOG(conn_logger_, error) << "Connection should be disowned";
        throw std::logic_error("Invalid connection state");
    }

    response_.setBody("").setCode(HttpCode::Continue);

    sendResponse(
        [cb]
        {
            cb();
        }
    );
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

std::pair<char*, size_t> Connection::mutable_body()
{
    return {request_buffer_.data() + offset_body_start_, offset_body_end_ - offset_body_start_};
}

} // namespace HTTP
} // namespace HTTPP
