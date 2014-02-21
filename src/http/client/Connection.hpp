/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_
# define HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_

# include <curl/curl.h>
# include <memory>
# include <future>
# include <vector>

# include <boost/log/trivial.hpp>
# include <boost/asio.hpp>

# include "httpp/http/Protocol.hpp"
# include "httpp/http/client/Request.hpp"
# include "httpp/http/client/Response.hpp"

namespace HTTPP {
namespace HTTP {
namespace client {

void parseCurlResponseHeader(const std::vector<char>& headers,
                             Response& response);

namespace detail {

struct Connection
{
    using ConnectionPtr = std::unique_ptr<Connection>;

    // duplicate alias from HttpClient
    using Future = std::future<Response>;
    using CompletionHandler = std::function<void (Future&&)>;

    Connection(boost::asio::io_service& service);
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    ~Connection();

    template <typename T>
    void conn_setopt(CURLoption opt, T t)
    {
        auto rc = curl_easy_setopt(handle, opt, t);
        if (rc != CURLE_OK)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Error setting curl option: " << curl_easy_strerror(rc);
            throw std::runtime_error("Cannot set option on curl");
        }
    }

    template <typename T>
    T conn_getinfo(CURLINFO info)
    {
        T data;
        auto rc = curl_easy_getinfo(handle, info, std::addressof(data));
        if (rc != CURLE_OK)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Can't get info: " << curl_easy_strerror(rc);
            throw std::runtime_error(curl_easy_strerror(rc));
        }

        return data;
    }


    void init();
    static ConnectionPtr createConnection(boost::asio::io_service& service);

    static size_t writefn(char* buffer, size_t size, size_t nmemb, void* userdata);
    static curl_socket_t opensocket(void* clientp,
                                    curlsocktype purpose,
                                    struct curl_sockaddr* address);

    void configureRequest(HTTPP::HTTP::Method method);

    void buildResponse(CURLcode code);
    void complete(std::exception_ptr ex = nullptr);

    CURL* handle;
    char error_buffer[CURL_ERROR_SIZE] = { 0 };
    boost::asio::ip::tcp::socket socket;

    client::Request request;
    client::Response response;

    std::promise<client::Response> promise;
    CompletionHandler completion_handler;
    bool expect_header = true;
    bool expect_continue = false;
    std::vector<char> header;
    std::vector<char> buffer;

    struct curl_slist* http_headers = nullptr;
};

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // !HTTPP_HTTP_CLIENT_DETAIL_CONNECTION_HPP_
