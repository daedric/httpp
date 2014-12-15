/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_CLIENT_REQUEST_HPP_
# define _HTTPP__HTTP_CLIENT_REQUEST_HPP_

# include <memory>
# include <string>
# include <vector>

namespace HTTPP
{

class HttpClient;

namespace HTTP
{
namespace client
{

namespace detail { struct Connection; }

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
    Request& url(const std::string& url);

    // unsafe, catenate current url + path
    Request& addToUrl(const std::string& path);

    // Allow insecure certificate
    Request& allowInsecure();

    Request& joinUrlPath(const std::string& dir, bool trailing_sep = false);
    Request& addUrlVariable(const std::string& var, const std::string& val);
    Request& followRedirect(bool b = true);
    Request& pushPostData(const std::string& name,
                          const std::string& value,
                          PostEncoding encoding = PostEncoding::Multipart);
    Request& setContent(const std::string& buffer);
    Request& addHeader(const std::string& k, const std::string& v);

    void clear();

private:
    std::string url_;
    bool follow_redirect_ = false;
    bool allow_insecure_ = false;

    std::vector<KV> query_params_;
    std::vector<KV> post_params_;
    std::vector<KV> http_headers_;

    std::string content_;
};

} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // _HTTPP__HTTP_CLIENT_REQUEST_HPP_

