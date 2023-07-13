/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Protocol.hpp"

#include <cstring>
#include <stdexcept>
#include <string>

namespace HTTPP
{
namespace HTTP
{
using namespace std::string_view_literals;

std::string_view to_string(Method method)
{
    switch (method)
    {
    default:
        return "UNKNOWN"sv;
    case Method::GET:
        return "GET"sv;
    case Method::POST:
        return "POST"sv;
    case Method::PUT:
        return "PUT"sv;
    case Method::HEAD:
        return "HEAD"sv;
    case Method::CONNECT:
        return "CONNECT"sv;
    case Method::TRACE:
        return "TRACE"sv;
    case Method::OPTIONS:
        return "OPTIONS"sv;
    case Method::DELETE_:
        return "DELETE"sv;
    }
}

#define APPLY_ON_METHOD(FN)                                                    \
    FN(HEAD, HEAD)                                                             \
    FN(GET, GET)                                                               \
    FN(POST, POST)                                                             \
    FN(PUT, PUT)                                                               \
    FN(DELETE, DELETE_)                                                        \
    FN(OPTIONS, OPTIONS)                                                       \
    FN(TRACE, TRACE)                                                           \
    FN(CONNECT, CONNECT)

Method method_from(std::string_view str)
{
#define fn(name, e)                                                            \
    if (str == #name##sv)                                                      \
    {                                                                          \
        return Method::e;                                                      \
    }
    APPLY_ON_METHOD(fn)
#undef fn

    throw std::runtime_error("Unknown method: " + std::string(str));
}

#define APPLY_ON_HTTP_CODE(FN)                                                 \
    FN(Continue)                                                               \
    FN(SwitchingProtocols)                                                     \
    FN(Ok)                                                                     \
    FN(Created)                                                                \
    FN(Accepted)                                                               \
    FN(NonAuthoritativeInformation)                                            \
    FN(NoContent)                                                              \
    FN(ResetContent)                                                           \
    FN(PartialContent)                                                         \
    FN(MultipleChoices)                                                        \
    FN(MovedPermanently)                                                       \
    FN(Redirect)                                                               \
    FN(SeeOther)                                                               \
    FN(NotModified)                                                            \
    FN(UseProxy)                                                               \
    FN(Unused)                                                                 \
    FN(TemporaryRedirect)                                                      \
    FN(BadRequest)                                                             \
    FN(Unauthorized)                                                           \
    FN(PaymentRequired)                                                        \
    FN(Forbidden)                                                              \
    FN(NotFound)                                                               \
    FN(MethodNotAllowed)                                                       \
    FN(NotAcceptable)                                                          \
    FN(ProxyAuthenticationRequired)                                            \
    FN(RequestTimeout)                                                         \
    FN(Conflict)                                                               \
    FN(Gone)                                                                   \
    FN(LengthRequired)                                                         \
    FN(PreconditionFailed)                                                     \
    FN(RequestEntityTooLarge)                                                  \
    FN(RequestUriTooLong)                                                      \
    FN(UnsupportedMediaType)                                                   \
    FN(RequestedRangeNotSatisfiable)                                           \
    FN(ExpectationFailed)                                                      \
    FN(InternalServerError)                                                    \
    FN(NotImplemented)                                                         \
    FN(BadGateway)                                                             \
    FN(ServiceUnavailable)                                                     \
    FN(GatewayTimeout)                                                         \
    FN(HttpVersionNotSupported)

std::string_view getDefaultMessage(HttpCode code)
{
    switch (code)
    {
    default:
        return "Unknown";

#define FN(code)                                                               \
    case HttpCode::code:                                                       \
        return #code##sv;
        APPLY_ON_HTTP_CODE(FN)
#undef FN
    }
}

} // namespace HTTP
} // namespace HTTPP
