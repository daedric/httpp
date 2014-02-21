/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <curl/curl.h>
#include <memory>
#include <future>
#include <iostream>
#include <vector>

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include "httpp/http/Protocol.hpp"
#include "httpp/http/client/Request.hpp"
#include "httpp/http/client/Response.hpp"

namespace HTTPP {
namespace HTTP {
namespace client {

void parseCurlResponseHeader(const std::vector<char>& headers,
                             Response& response);

namespace detail {

struct Connection
{
    using ConnectionPtr = std::unique_ptr<Connection>;

    Connection(boost::asio::io_service& service)
    : handle(curl_easy_init())
    , socket(service)
    {
        if (!handle)
        {
            throw std::runtime_error("Cannot initialize curl handle");
        }
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

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

    ~Connection()
    {
        if (http_headers)
        {
            curl_slist_free_all(http_headers);
        }

        if (handle)
        {
            curl_easy_cleanup(handle);
        }

    }

    void init()
    {
        curl_easy_reset(handle);
        conn_setopt(CURLOPT_WRITEFUNCTION, &writefn);
        conn_setopt(CURLOPT_WRITEDATA, this);
        conn_setopt(CURLOPT_ERRORBUFFER, this->error_buffer);
        conn_setopt(CURLOPT_PRIVATE, this);
        conn_setopt(CURLOPT_OPENSOCKETDATA, this);
        conn_setopt(CURLOPT_OPENSOCKETFUNCTION, &opensocket);

        if (http_headers)
        {
            curl_slist_free_all(http_headers);
        }

        promise = std::promise<client::Response>();
    }

    static ConnectionPtr createConnection(boost::asio::io_service& service)
    {
        return ConnectionPtr(new Connection(service));
    }

    static size_t writefn(char* buffer, size_t size, size_t nmemb, void* userdata)
    {
        Connection* conn = (Connection*)userdata;

        auto actual_size = size * nmemb;
        conn->buffer.insert(std::end(conn->buffer), buffer, buffer + actual_size);
        if (conn->expect_header)
        {

            auto begin = std::begin(conn->buffer);
            auto end = std::end(conn->buffer);
            auto it = std::search(begin,
                                  end,
                                  std::begin(HEADER_BODY_SEP),
                                  std::end(HEADER_BODY_SEP));

            if (it != end)
            {
                conn->expect_header = false;
                conn->header.insert(std::end(conn->header), begin, it + 2);
                conn->buffer.erase(begin, it + 4);
            }
        }

        return actual_size;
    }

    static curl_socket_t opensocket(void* clientp,
                                    curlsocktype purpose,
                                    struct curl_sockaddr* address)
    {
        Connection* conn = (Connection*)clientp;

        if (purpose == CURLSOCKTYPE_IPCXN)
        {
            boost::system::error_code ec;

            {
                // if a connection is recycled but the remote endpoint close
                // the connection we can't use the CLOSESOCKET hook since the
                // curl_multi_cleanup when cleaning the connection cache can
                // call it even if the Connection has been destroyed. Since the
                // curl_multi_remove_handle has been called, it is certainly a
                // bug in curl.

                boost::system::error_code ec;
                conn->socket.close(ec);
            }

            if (address->family == AF_INET)
            {
                conn->socket.open(boost::asio::ip::tcp::v4(), ec);
            }
            else
            {
                conn->socket.open(boost::asio::ip::tcp::v6(), ec);
            }

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error)
                    << "Cannot open a socket: " << ec.message();
                return CURL_SOCKET_BAD;
            }

            return conn->socket.native_handle();
        }

        return CURL_SOCKET_BAD;
    }

    void configureRequest(HTTPP::HTTP::Method method)
    {
        std::string method_str = to_string(method);
        conn_setopt(CURLOPT_CUSTOMREQUEST, method_str.data());

        if (!request.query_params_.empty())
        {
            std::string url = request.url_ + "?";

            bool add_sep = false;

            for (const auto& query_param : request.query_params_)
            {
                if (add_sep)
                {
                    url += "&";
                }
                url += query_param.first + "=" + query_param.second;
                add_sep = true;
            }

            conn_setopt(CURLOPT_URL, url.data());
        }
        else
        {
            conn_setopt(CURLOPT_URL, request.url_.data());
        }

        if (request.follow_redirect_)
        {
            conn_setopt(CURLOPT_FOLLOWLOCATION, 1L);
            conn_setopt(CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
        }

        if (!request.post_params_.empty() && !request.content_.empty())
        {
            throw std::runtime_error(
                "Cannot mix multipart and x-formurl-encoded post data");
        }

        struct curl_httppost* post = NULL;
        struct curl_httppost* last = NULL;
        if (!request.post_params_.empty())
        {
            for (const auto& e : request.post_params_)
            {
                curl_formadd(&post,
                             &last,
                             CURLFORM_COPYNAME,
                             e.first.data(),
                             CURLFORM_COPYCONTENTS,
                             e.second.data(),
                             CURLFORM_END);
            }

            conn_setopt(CURLOPT_HTTPPOST, post);
        }

        if (!request.content_.empty())
        {
            curl_formadd(&post,
                         &last,
                         CURLFORM_BUFFER,
                         "content",
                         CURLFORM_BUFFERPTR,
                         request.content_.data(),
                         CURLFORM_BUFFERLENGTH,
                         request.content_.size(),
                         CURLFORM_END);

            conn_setopt(CURLOPT_POSTFIELDS, request.content_.data());
            conn_setopt(CURLOPT_POSTFIELDSIZE, request.content_.size());
            conn_setopt(CURLOPT_POST, 1L);
        }

        conn_setopt(CURLOPT_NOSIGNAL, 1L);

        // only on curl > 7.25, disable it for now
        //conn_setopt(CURLOPT_TCP_KEEPALIVE, 1L);

        conn_setopt(CURLOPT_HEADER, 1L);

        if (!request.http_headers_.empty())
        {
            http_headers = nullptr;
            for (const auto& h : request.http_headers_)
            {
                http_headers = curl_slist_append(
                    http_headers, (h.first + ": " + h.second).data());
            }

            conn_setopt(CURLOPT_HTTPHEADER, http_headers);
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

    void buildResponse(CURLcode code)
    {
        if (code != CURLE_OK)
        {
            promise.set_exception(std::make_exception_ptr(
                std::runtime_error(curl_easy_strerror(code))));
            delete this;
            return;
        }
        try
        {
            response.code = static_cast<HTTPP::HTTP::HttpCode>(
                conn_getinfo<long>(CURLINFO_RESPONSE_CODE));

            request.connection_.reset(this);
            response.request = std::move(request);

            long redirection = 0;
            if (request.follow_redirect_)
            {
                redirection = conn_getinfo<long>(CURLINFO_REDIRECT_COUNT);
            }

            while (redirection)
            {
                header.clear();

                auto begin = std::begin(buffer);
                auto end = std::end(buffer);
                auto it = std::search(begin,
                                    end,
                                    std::begin(HEADER_BODY_SEP),
                                    std::end(HEADER_BODY_SEP));

                if (it != end)
                {
                    header.insert(std::end(header), begin, it + 2);
                    buffer.erase(begin, it + 4);
                }

                --redirection;
            }

            response.body.swap(buffer);
            parseCurlResponseHeader(header, response);
            promise.set_value(std::move(response));
        }
        catch (...)
        {
            promise.set_exception(std::current_exception());
        }
    }

    CURL* handle;
    char error_buffer[CURL_ERROR_SIZE] = { 0 };
    boost::asio::ip::tcp::socket socket;

    client::Request request;
    client::Response response;

    std::promise<client::Response> promise;
    bool expect_header = true;
    std::vector<char> header;
    std::vector<char> buffer;

    struct curl_slist* http_headers = nullptr;
};

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP
