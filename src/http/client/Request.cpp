/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/client/Request.hpp"
#include "httpp/utils/URL.hpp"
#include "Connection.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace client
{

Request::Request(){}
Request::~Request(){}

Request::Request(Request&& req) noexcept
{
    operator=(std::move(req));
}

Request& Request::operator=(Request&& req) noexcept
{
    connection_ = std::move(req.connection_);
    url_ = std::move(req.url_);
    follow_redirect_ = req.follow_redirect_;
    query_params_ = std::move(req.query_params_);
    post_params_ = std::move(req.post_params_);
    http_headers_ = std::move(req.http_headers_);
    content_ = std::move(req.content_);
    return *this;
}

Request& Request::url(const std::string& u)
{
    url_ = u;
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

    url_ += HTTPP::UTILS::encode(dir);

    if (trailing_sep)
    {
        url_ += '/';
    }

    return *this;
}

Request& Request::addUrlVariable(const std::string& var, const std::string& val)
{
    query_params_.emplace_back(var, UTILS::encode(val));
    return *this;
}

Request& Request::followRedirect(bool b)
{
    follow_redirect_ = b;
    return *this;
}

Request& Request::pushPostData(const std::string& name,
                               const std::string& value,
                               PostEncoding encoding)
{
    if (encoding == PostEncoding::FormUrlEncoded)
    {
        content_ += name + "=" + UTILS::encode(value) + '&';
    }
    else
    {
        post_params_.emplace_back(name, UTILS::encode(value));
    }

    return *this;
}

Request& Request::setContent(const std::string& buffer)
{
    content_ = buffer;
    return *this;
}

Request& Request::addHeader(const std::string& k, const std::string& v)
{
    http_headers_.emplace_back(k, v);
    return *this;
}

void Request::clear()
{
    url_.clear();
    follow_redirect_ = false;
    query_params_.clear();
    post_params_.clear();
    http_headers_.clear();
    content_.clear();
}

} // namespace client
} // namespace HTTP
} // namespace HTTPP
