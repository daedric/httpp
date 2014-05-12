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

public:
    static const size_t BUF_SIZE;
    using Callback = std::function<void ()>;

    Connection(HTTPP::HttpServer& handler,
               boost::asio::io_service& service,
               UTILS::ThreadPool& pool);
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
        bool expected = false;
        if (!is_owned_.compare_exchange_strong(expected, true))
        {
            throw std::logic_error("Invalid connection state");
        }

        if (buffer_.size())
        {
            assert(body_size >= buffer_.size());
            callable(boost::system::error_code(), buffer_.data(), buffer_.size());
            body_size -= buffer_.size();
            buffer_.clear();
        }

        if (!body_size)
        {
            disown();
            callable(boost::asio::error::eof, nullptr, 0);
            return;
        }

        auto size_to_read = std::min(BUF_SIZE, body_size);

        buffer_.resize(size_to_read);
        std::lock_guard<std::mutex> lock(mutex_);
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [size_to_read, callable, this](const boost::system::error_code& ec,
                                           size_t size) mutable
            {
                size_to_read -= size;

                if (size_to_read && ec == boost::asio::error::eof)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << "EOF detected while reading the body";
                    callable(boost::asio::error::eof, nullptr, 0);
                    return;
                }

                if (!size && ec)
                {
                    disown();
                    callable(ec, nullptr, 0);
                    return;
                }

                if (size)
                {
                    callable(ec, buffer_.data(), size);
                }

                if (size_to_read)
                {
                    readBody(size_to_read, callable);
                }
                else
                {
                    disown();
                    callable(boost::asio::error::eof, nullptr, 0);
                }
            });
    }

    void sendResponse();
    void sendContinue(Callback&& cb);
private:
    static void release(Connection* connection);

    void start();

    void disown() noexcept;
    bool shouldBeDeleted() const noexcept;
    void markToBeDeleted() noexcept;

    void cancel() noexcept;
    void close() noexcept;

    void read_request();
    void recycle();

    void sendResponse(Callback&& cb);

private:
    HTTPP::HttpServer& handler_;
    std::atomic_bool is_owned_ = { true };
    bool should_be_deleted_ = { false };
    UTILS::ThreadPool& pool_;
    std::vector<char> buffer_;
    size_t size_ = 0;

    std::mutex mutex_;
    boost::asio::ip::tcp::socket socket_;
    Response response_;
};

} // namespace HTTP
} // namespace HTTPP
#endif // ! _HTTPP_HTPP_CONNECTION_HPP_
