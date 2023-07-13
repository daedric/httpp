/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/client/Request.hpp"

#include <chrono>

#include "Connection.hpp"
#include "httpp/utils/URL.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace client
{

Request& Request::url(std::string_view u)
{
    url_.assign(u.begin(), u.end());
    return *this;
}

Request& Request::addToUrl(const std::string& path)
{
    url_ += path;
    return *this;
}

Request& Request::joinUrlPath(const std::string& dir, bool trailing_sep)
{
    if (!url_.empty())
    {
        if (url_[url_.size() - 1] != '/')
        {
            url_ += '/';
        }
    }

    url_ += HTTPP::UTILS::url_encode(dir);

    if (trailing_sep)
    {
        url_ += '/';
    }

    return *this;
}

Request& Request::addUrlVariable(std::string var, std::string val)
{
    query_params_.emplace_back(std::move(var), UTILS::url_encode(val));
    return *this;
}

Request& Request::followRedirect(bool b)
{
    follow_redirect_ = b;
    return *this;
}

Request& Request::setTimeout(std::chrono::milliseconds timeout)
{
    timeout_ = timeout;
    return *this;
}

Request& Request::pushPostData(std::string name, std::string value, PostEncoding encoding)
{
    if (encoding == PostEncoding::FormUrlEncoded)
    {
        content_ += std::move(name) + "=" + UTILS::url_encode(value) + '&';
    }
    else
    {
        post_params_.emplace_back(std::move(name), UTILS::url_encode(value));
    }

    return *this;
}

Request& Request::setContent(std::string buffer)
{
    content_ = std::move(buffer);
    return *this;
}

Request& Request::addHeader(std::string k, std::string v)
{
    http_headers_.emplace_back(std::move(k), std::move(v));
    return *this;
}

Request& Request::allowInsecure()
{
    allow_insecure_ = true;
    return *this;
}

void Request::clear()
{
    url_.clear();
    timeout_ = std::chrono::milliseconds::zero();
    follow_redirect_ = false;
    allow_insecure_ = false;
    query_params_.clear();
    post_params_.clear();
    http_headers_.clear();
    content_.clear();
}

} // namespace client
} // namespace HTTP
} // namespace HTTPP
