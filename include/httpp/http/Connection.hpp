/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_CONNECTION_HPP_
# define _HTTPP_HTPP_CONNECTION_HPP_

# include <string>
# include <boost/asio.hpp>

# include "Response.hpp"

namespace HTTPP
{

class HttpServer;

namespace UTILS { class ThreadPool; }

namespace HTTP
{

class Connection
{
    friend class ::HTTPP::HttpServer;

public:
    static const size_t BUF_SIZE;

    Connection(HTTPP::HttpServer& handler,
               boost::asio::io_service& service,
               UTILS::ThreadPool& pool);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    static void disconnect(Connection* connection)
    {
        delete connection;
    }

    std::string source() const;

    Response& response()
    {
        return response_;
    }

    const Response& response() const
    {
        return response_;
    }

    template <typename Callable>
    void readBody(size_t body_size, Callable&& callable)
    {
        if (buffer_.size())
        {
            assert(body_size >= buffer_.size());
            callable(boost::system::error_code(), buffer_.data(), buffer_.size());
            body_size -= buffer_.size();
            buffer_.clear();
        }

        auto size_to_read = std::min(BUF_SIZE, body_size);
        buffer_.reserve(size_to_read);
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [size_to_read, callable, this](const boost::system::error_code& ec,
                                           size_t size) mutable
            {
                callable(ec, buffer_.data(), size);
                size_to_read -= size;
                if (size)
                {
                    readBody(size, callable);
                }
                else
                {
                    callable(boost::asio::error::eof, nullptr, 0);
                }
            });
    }

    void sendResponse();

private:
    void start();
    void read_request();
    void recycle();

private:
    HTTPP::HttpServer& handler_;
    UTILS::ThreadPool& pool_;
    std::vector<char> buffer_;
    size_t size_ = 0;
    boost::asio::ip::tcp::socket socket_;
    Response response_;
};

} // namespace HTTP
} // namespace HTTPP
#endif // ! _HTTPP_HTPP_CONNECTION_HPP_
