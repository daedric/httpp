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
                       UTILS::ThreadPool& pool)
: handler_(handler)
, pool_(pool)
, socket_(service)
{}

Connection::~Connection()
{
    BOOST_LOG_TRIVIAL(debug) << "Disconnect client";
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
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
    size_ = 0;
    buffer_.resize(BUF_SIZE);
    read_request();
}

void Connection::read_request()
{
    if (Parser::isComplete(buffer_.data(), size_))
    {
        UTILS::VectorStreamBuf buf(buffer_, size_);
        std::istream is(std::addressof(buf));
        Request request;
        if (Parser::parse(is, request))
        {
            BOOST_LOG_TRIVIAL(debug) << "Received a request from: " << source()
                << ": " << request;

            buf.shrinkVector();
            handler_.connection_notify_request(this, std::move(request));
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning)
                << "Invalid request received from: " << source();

            response_ = Response(
                    HttpCode::BadRequest,
                    std::string("An error occured in the request parsing indicating an error"));
            response_.connectionShouldBeClosed(true);

            sendResponse();
        }
    }
    else
    {
        if (size_ >= buffer_.size())
        {
            buffer_.resize(buffer_.capacity() + BUF_SIZE);
        }

        char* data = buffer_.data();
        data += size_;

        socket_.async_read_some(
            boost::asio::buffer(data, buffer_.capacity() - size_),
            [&](boost::system::error_code const& ec, size_t size)
            {
                if (ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        handler_.connection_error(this, ec);
                    }
                    return ;
                }

                this->size_ += size;
                read_request();
            }
        );
    }
}

void Connection::sendResponse()
{
    response_.sendResponse(socket_,
                           [this](boost::system::error_code const& ec, size_t)
                           {
                                if (ec)
                                {
                                    if (ec != boost::asio::error::operation_aborted)
                                    {
                                        handler_.connection_error(this, ec);
                                        return;
                                    }

                                    pool_.post([this]
                                               {
                                                   BOOST_LOG_TRIVIAL(debug) << "Disconnect client after an error "
                                                                               "occured";
                                                   disconnect(this);
                                               });
                                    return;
                                }

                                this->recycle();
                           });
}

void Connection::recycle()
{
    if (response_.connectionShouldBeClosed())
    {
        pool_.post([this]
                   { disconnect(this); });
    }
    else
    {
        handler_.connection_recycle(this);
    }
}

} // namespace HTTP
} // namespace HTTPP

