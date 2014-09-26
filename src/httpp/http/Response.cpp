/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Response.hpp"

#include <stdexcept>

namespace HTTPP
{
namespace HTTP
{

char const Response::HTTP_DELIMITER[] = { '\r', '\n'};
char const Response::HEADER_SEPARATOR[] = { ':', ' '};
char const Response::END_OF_STREAM_MARKER[] = { '0', '\r', '\n', '\r', '\n'};

Response::Response()
: Response(HttpCode::Ok)
{
}

Response::Response(HttpCode code)
: code_(code)
, body_(getDefaultMessage(code_))
{
}

Response::Response(HttpCode code, const std::string& body)
: code_(code)
, body_(body)
{
}

Response::Response(HttpCode code, std::string&& body)
: code_(code)
, body_(std::move(body))
{
}

Response::Response(HttpCode code, ChunkedResponseCallback&& callback)
: code_(code),
  chunkedBodyCallback_(std::move(callback))
{
}

Response& Response::addHeader(const std::string& k, const std::string& v)
{
    if (k == "Content-Length")
    {
        throw std::invalid_argument("Content-Length header should not be set.");
    }

    if (k == "Transfer-Encoding")
    {
        throw std::invalid_argument("Transfer-Encoding header should not be set.");
    }

    if (k.empty() || v.empty())
    {
        throw std::invalid_argument("Attempting to addHeader with an empty key or value");
    }

    headers_.emplace_back(k, v);
    return *this;
}

Response& Response::setBody(const std::string& body)
{
    chunkedBodyCallback_ = 0;
    body_ = body;
    return *this;
}

Response& Response::setBody(ChunkedResponseCallback && callback)
{
    if (callback)
    {
        body_.clear();
        chunkedBodyCallback_ = std::move(callback);
        return *this;
    }
    else
    {
        throw std::invalid_argument("Setting chunked response body to an empty callback");
    }
}

} // namespace HTTP
} // namespace HTTPP
