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
#define _HTTPP_HTPP_CONNECTION_HPP_

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <commonpp/core/LoggingInterface.hpp>
#include <commonpp/thread/ThreadPool.hpp>

#include "Request.hpp"
#include "Response.hpp"
#include "helper/ReadWholeRequest.hpp"

namespace HTTPP
{

class HttpServer;

namespace HTTP
{

namespace connection_detail
{
FWD_DECLARE_LOGGER(conn_logger_, commonpp::core::Logger);
} // namespace connection_detail

class Connection
{
    friend class ::HTTPP::HttpServer;

    using DefaultSocket =
        boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type>;
    using SSLSocket = boost::asio::ssl::stream<DefaultSocket&>;
    using ThreadPool = commonpp::thread::ThreadPool;

public:
    static constexpr size_t BUF_SIZE = BUFSIZ;
    using Callback = std::function<void()>;

    Connection(
        HTTPP::HttpServer& handler,
        boost::asio::io_service& service,
        boost::asio::ssl::context* ctx = nullptr
    );

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
    void read(size_t size, char* buffer, Callable callable)
    {
        if (!own())
        {
            throw std::logic_error("Invalid connection state");
        }

        auto already_read = request_buffer_.size() - offset_body_start_;
        if (already_read)
        {
            if (size <= already_read)
            {
                offset_body_end_ = offset_body_start_ + size;
                std::copy(
                    request_buffer_.begin() + offset_body_start_,
                    request_buffer_.begin() + offset_body_end_,
                    buffer
                );
                disown();
                callable(boost::system::error_code());
                return;
            }
            std::copy(
                request_buffer_.begin() + offset_body_start_, request_buffer_.end(), buffer
            );
            size -= already_read;
            request_buffer_.resize(offset_body_start_);
        }

        offset_body_end_ = offset_body_start_;
        async_read(
            boost::asio::buffer(buffer + already_read, size),
            [callable = std::move(callable), this](const boost::system::error_code& ec, size_t) mutable
            {
                disown();

                if (ec)
                {
                    LOG(connection_detail::conn_logger_, error)
                        << "Error detected while reading the body";
                    callable(ec);
                    return;
                }

                callable(boost::system::error_code());
            }
        );
    }

    template <typename Callable, size_t BUFFER_SIZE = BUF_SIZE>
    void read(size_t body_size, Callable callable)
    {
        if (!own())
        {
            throw std::logic_error("Invalid connection state");
        }

        auto already_read = request_buffer_.size() - offset_body_start_;
        if (already_read)
        {
            auto body_start = request_buffer_.data() + offset_body_start_;
            if (body_size <= already_read)
            {
                offset_body_end_ = offset_body_start_ + body_size;
                callable(boost::system::error_code(), body_start, already_read);
                body_size = 0;
            }
            else
            {
                callable(boost::system::error_code(), body_start, already_read);
                body_size -= already_read;
            }
        }

        if (!body_size)
        {
            disown();
            callable(boost::system::error_code(), nullptr, 0);
            return;
        }

        offset_body_end_ = offset_body_start_;
        auto buf_size = std::min(BUFFER_SIZE, body_size);

        auto capacity = request_buffer_.capacity() - offset_body_start_;
        if (capacity < buf_size)
        {
            request_buffer_.reserve(offset_body_start_ + buf_size + 1);
            reparse();
        }

        request_buffer_.resize(request_buffer_.capacity(), 0);
        async_read_some(
            boost::asio::buffer(request_buffer_.data() + offset_body_start_, buf_size),
            [body_size,
             callable = std::move(callable),
             this](const boost::system::error_code& ec, size_t size) mutable
            {
                disown();

                if (ec)
                {
                    LOG(connection_detail::conn_logger_, error)
                        << "Error detected while reading the body";
                    callable(ec, nullptr, 0);
                    return;
                }

                request_buffer_.resize(offset_body_start_ + size);
                read(body_size, std::move(callable));
            }
        );
    }

    template <typename Callable>
    void read_whole(size_t body_size, Callable callable)
    {
        if (!own())
        {
            throw std::logic_error("Invalid connection state");
        }

        auto already_read = request_buffer_.size() - offset_body_start_;
        offset_body_end_ = offset_body_start_ + body_size;
        if (already_read)
        {
            if (body_size <= already_read)
            {
                disown();
                callable(boost::system::error_code());
                return;
            }
        }

        if (!body_size)
        {
            disown();
            callable(boost::system::error_code());
            return;
        }

        auto missing = body_size - already_read;

        auto capacity = offset_body_end_ + 1;
        if (request_buffer_.capacity() < capacity)
        {
            request_buffer_.reserve(capacity);
            // pointer might have changed, so we need to reconstruct header,
            // path, etc.
            reparse();
        }

        request_buffer_.resize(capacity, 0);
        async_read(
            boost::asio::buffer(request_buffer_.data() + offset_body_end_ - missing, missing),
            [callable = std::move(callable),
             this](const boost::system::error_code& ec, size_t size) mutable
            {
                disown();

                if (ec)
                {
                    LOG(connection_detail::conn_logger_, error)
                        << "Error detected while reading the body";
                    callable(ec);
                    return;
                }

                request_buffer_.resize(offset_body_end_);
                callable(boost::system::error_code());
            }
        );
    }

    void sendResponse();
    void sendContinue(Callback&& cb);
    std::pair<char*, size_t> mutable_body();

private:
    void reparse();

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

    template <typename... Args>
    void async_read_some(Args&&... a)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ssl_socket_)
        {
            ssl_socket_->async_read_some(std::forward<Args>(a)...);
        }
        else
        {
            socket_.async_read_some(std::forward<Args>(a)...);
        }
    }

    template <typename... Args>
    void async_read(Args&&... a)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ssl_socket_)
        {
            boost::asio::async_read(*ssl_socket_, std::forward<Args>(a)...);
        }
        else
        {
            boost::asio::async_read(socket_, std::forward<Args>(a)...);
        }
    }

    void sendResponse(Callback&& cb);

private:
    HTTPP::HttpServer& handler_;
    // On construction, the HttpServer is the owner
    std::atomic_bool is_owned_ = {false};
    bool should_be_deleted_ = {false};
    std::vector<char> request_buffer_;
    size_t size_ = 0;
    size_t offset_body_start_ = 0;
    size_t offset_body_end_ = 0;

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
