/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_HTTP_CODE_HPP_
# define _HTTPP_HTPP_HTTP_CODE_HPP_

# include <string>

namespace HTTPP
{
namespace HTTP
{

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

#endif // ! _HTTPP_HTPP_HTTP_CODE_HPP_
