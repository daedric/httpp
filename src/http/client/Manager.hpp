/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_
# define HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_

# include <curl/curl.h>
# include <memory>
# include <stdexcept>

# include <boost/log/trivial.hpp>

# include "httpp/http/Protocol.hpp"
# include "httpp/utils/ThreadPool.hpp"

namespace HTTPP {
namespace HTTP {
namespace client {

namespace detail {

struct Connection;

struct Manager
{
    using Method = HTTPP::HTTP::Method;
    using ConnectionPtr = std::unique_ptr<Connection>;

    Manager(UTILS::ThreadPool& pool);
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    ~Manager();

    template <typename T>
    void manager_setopt(CURLMoption opt, T t)
    {
        auto rc = curl_multi_setopt(handler, opt, t);
        if (rc != CURLM_OK)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error setting curl option: " << curl_multi_strerror(rc);
            throw std::runtime_error("Cannot set option on curl");
        }
    }

    void timer_cb(const boost::system::error_code& error);
    static int curl_timer_cb(CURLM*, long timeout_ms, void* userdata);

    static int sock_cb(CURL* easy,
                       curl_socket_t s,
                       int what,
                       void* multi_private,
                       void* socket_private);

    void checkHandles();
    void performOp(Connection* connection, int action);
    void poll(Connection* connection, int action);

    void handleRequest(Method method, ConnectionPtr connection);

    CURLM* handler;
    UTILS::ThreadPool& pool;
    UTILS::ThreadPool::Strand strand;
    UTILS::ThreadPool::Timer timer;
};
} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // ! HTTPP_HTTP_CLIENT_DETAIL_MANAGER_HPP_
