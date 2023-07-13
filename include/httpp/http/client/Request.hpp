/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_CLIENT_REQUEST_HPP_
#define _HTTPP__HTTP_CLIENT_REQUEST_HPP_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace HTTPP
{

class HttpClient;

namespace HTTP
{
namespace client
{

namespace detail
{
struct Connection;
} // namespace detail

class Request
{
    friend class HTTPP::HttpClient;
    friend struct ::HTTPP::HTTP::client::detail::Connection;

public:
    using KV = std::pair<std::string, std::string>;

    enum class PostEncoding
    {
        Multipart,
        FormUrlEncoded
    };

public:
    Request& url(std::string_view url);

    // unsafe, catenate current url + path
    Request& addToUrl(const std::string& path);

    // Allow insecure certificate
    Request& allowInsecure();

    Request& setTimeout(std::chrono::milliseconds timeout);
    Request& joinUrlPath(const std::string& dir, bool trailing_sep = false);
    Request& addUrlVariable(const std::string var, const std::string val);
    Request& followRedirect(bool b = true);
    Request& pushPostData(
        std::string name, std::string value, PostEncoding encoding = PostEncoding::Multipart
    );
    Request& setContent(std::string buffer);
    Request& addHeader(std::string k, std::string v);

    void clear();

private:
    std::string url_;
    bool follow_redirect_ = false;
    bool allow_insecure_ = false;

    std::vector<KV> query_params_;
    std::vector<KV> post_params_;
    std::vector<KV> http_headers_;

    std::string content_;
    std::chrono::milliseconds timeout_{0};
};

} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // _HTTPP__HTTP_CLIENT_REQUEST_HPP_
