/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_CONNECTION_HPP_
# define _HTTPP_HTPP_CONNECTION_HPP_

# include <atomic>
# include <mutex>
# include <string>
# include <functional>
# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>
# include <commonpp/thread/ThreadPool.hpp>
# include <commonpp/core/LoggingInterface.hpp>

# include "helper/ReadEverything.hpp"
# include "Response.hpp"
# include "Request.hpp"

namespace HTTPP
{

class HttpServer;

namespace HTTP
{

namespace connection_detail
{
FWD_DECLARE_LOGGER(conn_logger_, commonpp::core::Logger);
}

class Connection
{
    friend class ::HTTPP::HttpServer;

    using DefaultSocket = boost::asio::ip::tcp::socket;
    using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;
    using ThreadPool = commonpp::thread::ThreadPool;

public:
    static const size_t BUF_SIZE;
    using Callback = std::function<void ()>;

    Connection(HTTPP::HttpServer& handler,
               boost::asio::io_service& service,
               boost::asio::ssl::context* ctx = nullptr);

private:
    ~Connection();

public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    static void releaseFromHandler(Connection* connection);
    std::string source() const;

    Response& response() noexcept
    {
        return response_;
    }

    const Response& response() const noexcept
    {
        return response_;
    }

    const Request& request() const noexcept
    {
        return request_;
    }

    template <typename Callable>
    void readBody(size_t body_size, Callable&& callable)
    {
        if (!own())
        {
            throw std::logic_error("Invalid connection state");
        }

        if (not body_buffer_.empty())
        {
            if (body_size <= body_buffer_.size())
            {
                callable(boost::system::error_code(), body_buffer_.data(),
                         body_size);
                body_buffer_.erase(begin(body_buffer_),
                                   begin(body_buffer_) + body_size);
                body_size = 0;
            }
            else
            {
                callable(boost::system::error_code(), body_buffer_.data(),
                         body_buffer_.size());
                body_size -= body_buffer_.size();
                body_buffer_.clear();
            }
        }

        if (!body_size)
        {
            disown();
            callable(boost::system::error_code(), nullptr, 0);
            return;
        }

        auto buf_size = std::min(BUF_SIZE, body_size);

        body_buffer_.resize(buf_size);

        async_read_some(boost::asio::buffer(body_buffer_),
                        [body_size, callable, this](
                            const boost::system::error_code& ec, size_t size) {
                            disown();

                            if (ec)
                            {
                                LOG(connection_detail::conn_logger_, error)
                                    << "Error detected while reading the body";
                                callable(ec, nullptr, 0);
                                return;
                            }

                            body_buffer_.resize(size);
                            readBody(body_size, callable);
                        });
    }

    void sendResponse();
    void sendContinue(Callback&& cb);

private:
    static void release(Connection* connection);

    void start();

    void disown();
    bool own() noexcept;
    bool shouldBeDeleted() const noexcept;
    void markToBeDeleted();

    void cancel() noexcept;
    void close() noexcept;

    void read_request();
    void recycle();

    template <typename Buffer, typename Handler>
    void async_read_some(Buffer&& buffer, Handler&& handler)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ssl_socket_)
        {
            ssl_socket_->async_read_some(std::forward<Buffer>(buffer),
                                         std::forward<Handler>(handler));
        }
        else
        {
            socket_.async_read_some(std::forward<Buffer>(buffer),
                                    std::forward<Handler>(handler));
        }
    }

    void sendResponse(Callback&& cb);

private:
    HTTPP::HttpServer& handler_;
    // On construction, the HttpServer is the owner
    std::atomic_bool is_owned_ = { false };
    bool should_be_deleted_ = { false };
    std::vector<char> request_buffer_;
    std::vector<char> body_buffer_;
    size_t size_ = 0;

    std::mutex mutex_;

    DefaultSocket socket_;
    bool need_handshake_ = true;
    std::unique_ptr<SSLSocket> ssl_socket_;

    Request request_;
    Response response_;
};

} // namespace HTTP
} // namespace HTTPP
#endif // ! _HTTPP_HTPP_CONNECTION_HPP_
