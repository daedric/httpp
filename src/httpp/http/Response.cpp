/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Response.hpp"

#include <cstring>
#include <stdexcept>

namespace HTTPP
{
namespace HTTP
{
char const Response::HTTP_START[] = {
    'H', 'T', 'T', 'P', '/', '1', '.', '1', ' ',
};
char const Response::SPACE[] = {
    ' ',
};
char const Response::HTTP_DELIMITER[] = {
    '\r',
    '\n',
};
char const Response::HEADER_SEPARATOR[] = {
    ':',
    ' ',
};
char const Response::END_OF_STREAM_MARKER[] = {
    '0', '\r', '\n', '\r', '\n',
};

Response::Response(HttpCode code)
{
    setCode(code);
    setBody(getDefaultMessage(code_));
}

Response::Response(HttpCode code, const std::string_view& body)
{
    setCode(code);
    setBody(body);
}

Response::Response(HttpCode code, ChunkedResponseCallback&& callback)
: chunkedBodyCallback_(std::move(callback))
{
    setCode(code);
}

void Response::clear()
{
    setCode(HttpCode::Ok);
    setBody("");

    should_be_closed_ = false;
    chunkedBodyCallback_ = nullptr;
    current_chunk_.clear();
    current_chunk_header_.clear();
    status_string_.clear();
    headers_.clear();
}

Response& Response::addHeader(std::string k, std::string v)
{
    if (k == "Content-Length")
    {
        throw std::invalid_argument("Content-Length header should not be set.");
    }

    if (k == "Transfer-Encoding")
    {
        throw std::invalid_argument(
            "Transfer-Encoding header should not be set.");
    }

    if (k.empty() || v.empty())
    {
        throw std::invalid_argument(
            "Attempting to addHeader with an empty key or value");
    }

    headers_.emplace_back(std::move(k), std::move(v));
    return *this;
}

Response& Response::setBody(const std::string_view& body)
{
    chunkedBodyCallback_ = nullptr;
    body_.reserve(body.size());
    body_.assign(std::begin(body), std::end(body));
    return *this;
}

Response& Response::setBody(ChunkedResponseCallback&& callback)
{
    if (callback)
    {
        body_.clear();
        chunkedBodyCallback_ = std::move(callback);
        return *this;
    }
    else
    {
        throw std::invalid_argument(
            "Setting chunked response body to an empty callback");
    }
}

} // namespace HTTP
} // namespace HTTPP
