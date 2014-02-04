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
# include <boost/asio.hpp>

# include "http/Connection.hpp"
# include "http/Request.hpp"
# include "utils/ThreadPool.hpp"

namespace HTTPP
{

class HttpServer
{
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using AcceptorPtr = std::shared_ptr<Acceptor>;

public:
    using ConnectionPtr = HTTP::Connection*; //std::shared_ptr<HTTP::Connection>;
    using SinkCb = std::function<void (ConnectionPtr, HTTP::Request&&)>;
    using ThreadInit = UTILS::ThreadPool::ThreadInit;

public:
    HttpServer(size_t threads = 1);
    ~HttpServer();

    void bind(const std::string& address, const std::string& port = "8000");
    void setSink(SinkCb cb)
    {
        sink_ = cb;
    }

    void start(ThreadInit fct = ThreadInit());
    void stop();

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
    void connection_error(ConnectionPtr connection,
                          const boost::system::error_code& err);

    void connection_notify_request(ConnectionPtr connection,
                                   HTTP::Request&& request);
    void connection_recycle(ConnectionPtr connection);

private:
    bool running_ = false;
    boost::asio::io_service service_;
    UTILS::ThreadPool pool_;
    std::vector<AcceptorPtr> acceptors_;
    SinkCb sink_;
};

} // namespace HTTPP

#endif // !_HTTPP__HTTP_SERVER_HPP_
