/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_HTTP_PROTOCOL_HPP_
# define HTTPP_HTTP_PROTOCOL_HPP_

# include <string>
# include <boost/utility/string_ref.hpp>
# include <commonpp/core/string/std_tostring.hpp>

# include <httpp/detail/config.hpp>

namespace HTTPP
{
namespace HTTP
{

using KV = std::pair<std::string, std::string>;
using KVRef = std::pair<boost::string_ref, boost::string_ref>;
using Header = KV;
# if HTTPP_PARSER_BACKEND == HTTPP_RAGEL_BACKEND
using HeaderRef = KVRef;
# else
using HeaderRef = KV;
# endif

static char const HEADER_BODY_SEP[] = { '\r', '\n', '\r', '\n' };

enum class Method
{
    HEAD    ,
    GET     ,
    POST    ,
    PUT     ,
    DELETE_ , // '_' for msvc workaround
    OPTIONS ,
    TRACE   ,
    CONNECT
};

std::string to_string(Method method);
Method method_from(const std::string& str);
Method method_from(const char* str);

enum class HttpCode : unsigned int
{
    Continue = 100,
    SwitchingProtocols = 101,

    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,

    Ambiguous = 300,
    MultipleChoices = 300,
    Moved = 301,
    MovedPermanently = 301,
    Found = 302,
    Redirect = 302,
    RedirectMethod = 303,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    Unused = 306,
    RedirectKeepVerb = 307,
    TemporaryRedirect = 307,

    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    RequestEntityTooLarge = 413,
    RequestUriTooLong = 414,
    UnsupportedMediaType = 415,
    RequestedRangeNotSatisfiable = 416,
    ExpectationFailed = 417,

    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505
};

const char* getDefaultMessage(HttpCode code);

} // namespace HTTP
} // namespace HTTPP

#endif // !HTTPP_HTTP_PROTOCOL_HPP_

