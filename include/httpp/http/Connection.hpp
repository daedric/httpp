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

# include <atomic>
# include <mutex>
# include <string>
# include <functional>
# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>
# include <boost/log/trivial.hpp>

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

    using DefaultSocket = boost::asio::ip::tcp::socket;
    using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;

public:
    static const size_t BUF_SIZE;
    using Callback = std::function<void ()>;

    Connection(HTTPP::HttpServer& handler,
               boost::asio::io_service& service,
               UTILS::ThreadPool& pool,
               boost::asio::ssl::context* ctx = nullptr);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    static void releaseFromHandler(Connection* connection);
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
        if (!own())
        {
            throw std::logic_error("Invalid connection state");
        }

        if (buffer_.size())
        {
            if (body_size <= buffer_.size())
            {
                callable(boost::system::error_code(), buffer_.data(), body_size);
                buffer_.erase(begin(buffer_), begin(buffer_) + body_size);
                body_size = 0;
            }
            else
            {
                callable(boost::system::error_code(), buffer_.data(), buffer_.size());
                body_size -= buffer_.size();
                buffer_.clear();
            }
        }

        if (!body_size)
        {
            disown();
            callable(boost::asio::error::eof, nullptr, 0);
            return;
        }

        auto buf_size = std::min(BUF_SIZE, body_size);

        buffer_.resize(buf_size);
        std::lock_guard<std::mutex> lock(mutex_);

        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [body_size, callable, this](const boost::system::error_code& ec,
                                        size_t size)
            {
                disown();

                if (ec)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << "Error detected while reading the body";
                    callable(ec, nullptr, 0);
                    return;
                }

                buffer_.resize(size);
                readBody(body_size, callable);
            });
    }

    void sendResponse();
    void sendContinue(Callback&& cb);

private:
    static void release(Connection* connection);

    void start();

    void disown() noexcept;
    bool own() noexcept;
    bool shouldBeDeleted() const noexcept;
    void markToBeDeleted() noexcept;

    void cancel() noexcept;
    void close() noexcept;

    void read_request();
    void recycle();

    template <typename Buffer, typename Handler>
    void async_read_some(Buffer&& buffer, Handler&& handler);

    void sendResponse(Callback&& cb);

private:
    HTTPP::HttpServer& handler_;
    std::atomic_bool is_owned_ = { true };
    bool should_be_deleted_ = { false };
    UTILS::ThreadPool& pool_;
    std::vector<char> buffer_;
    size_t size_ = 0;

    std::mutex mutex_;

    DefaultSocket socket_;
    bool need_handshake_ = true;
    std::unique_ptr<SSLSocket> ssl_socket_;

    Response response_;
};

} // namespace HTTP
} // namespace HTTPP
#endif // ! _HTTPP_HTPP_CONNECTION_HPP_
