/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_SERVER_HPP_
# define _HTTPP__HTTP_SERVER_HPP_

# include <string>
# include <vector>
# include <memory>
# include <mutex>
# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>

# include "http/Connection.hpp"
# include "http/Request.hpp"
# include "utils/ThreadPool.hpp"

namespace HTTPP
{

class HttpServer
{
public:
    struct SSLContext
    {
        std::string cert_file;
        std::string key_file;
        std::string dh_file;

        std::string cert_buffer;
        std::string key_buffer;
        std::string dh_buffer;
    };

private:
    struct Acceptor;
    using AcceptorPtr = std::shared_ptr<Acceptor>;

public:
    using ConnectionPtr = HTTP::Connection*; //std::shared_ptr<HTTP::Connection>;
    using SinkCb = std::function<void (ConnectionPtr, HTTP::Request&&)>;
    using ThreadInit = UTILS::ThreadPool::ThreadInit;

public:
    HttpServer(size_t threads = 1);
    HttpServer(UTILS::ThreadPool& pool);
    ~HttpServer();

    void bind(const std::string& address, const std::string& port = "8000");
    void bind(const std::string& address,
              SSLContext ctx,
              const std::string& port = "443");

    void setSink(SinkCb cb)
    {
        sink_ = cb;
    }

    int getNbConnection() const noexcept
    { return connection_count_; }

    void start(ThreadInit fct = ThreadInit());
    void stop();

    void stopListeners();

private:
    void start_accept(AcceptorPtr acceptor);
    void accept_callback(const boost::system::error_code& error,
                         AcceptorPtr acceptor,
                         ConnectionPtr connection);

    static AcceptorPtr bind(boost::asio::io_service&,
                            const std::string& address,
                            const std::string& port);

private: // called by Connection
    friend class ::HTTPP::HTTP::Connection;

    void mark(ConnectionPtr connection);
    void destroy(ConnectionPtr connection, bool release = true);

    void connection_error(ConnectionPtr connection,
                          const boost::system::error_code& err);

    void connection_notify_request(ConnectionPtr connection,
                                   HTTP::Request&& request);
    void connection_recycle(ConnectionPtr connection);

private:
    bool running_ = false;
    std::shared_ptr<UTILS::ThreadPool> pool_;
    std::atomic_int running_acceptors_ = { 0 };
    std::atomic_int connection_count_ = { 0 };
    std::vector<AcceptorPtr> acceptors_;
    SinkCb sink_;

    std::mutex connections_mutex_;
    std::vector<ConnectionPtr> connections_;
};

} // namespace HTTPP

#endif // !_HTTPP__HTTP_SERVER_HPP_
