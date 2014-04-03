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

Response& Response::addHeader(const std::string& k, const std::string& v)
{
    if (k == "Content-Length")
    {
        throw std::invalid_argument("Content-Lenght should not be set");
    }

    if (k.empty() || v.empty())
    {
        throw std::invalid_argument("key nor value can be empty");
    }

    headers_.emplace_back(k, v);
    return *this;
}

Response& Response::setBody(const std::string& body)
{
    body_ = body;
    return *this;
}

} // namespace HTTP
} // namespace HTTPP
