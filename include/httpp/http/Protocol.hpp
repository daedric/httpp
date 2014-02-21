/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_PROTOCOL_HPP_
# define HTTPP_HTTP_PROTOCOL_HPP_

# include <string>

namespace HTTPP
{
namespace HTTP
{

using KV = std::pair<std::string, std::string>;
using Header = KV;

static char const HEADER_BODY_SEP[] = { '\r', '\n', '\r', '\n' };

enum class Method
{
    HEAD,
    GET,
    POST,
    PUT,
    DELETE_, // '_' for msvc workaround
    OPTIONS,
    TRACE,
    CONNECT
};

std::string to_string(Method method);

enum class HttpCode : unsigned int
{
    Continue = 100,

    Ok = 200,
    Created = 201,
    Accepted = 202,
    NoContent = 204,

    MultipleChoice = 300,
    MovedPermentaly = 301,
    MovedTemporarily = 302,
    NotModified = 304,

    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,

    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    HttpVersionNotSupported = 505
};

std::string getDefaultMessage(HttpCode code);

} // namespace HTTP
} // namespace HTTPP

#endif // !HTTPP_HTTP_PROTOCOL_HPP_

