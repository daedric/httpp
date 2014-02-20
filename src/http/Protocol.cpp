/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Protocol.hpp"
#include <string>

namespace HTTPP
{
namespace HTTP
{

std::string to_string(Method method)
{
    switch (method)
    {
        default:
            return "UNKNOWN";
        case Method::GET:
            return "GET";
        case Method::POST:
            return "POST";
        case Method::PUT:
            return "PUT";
        case Method::HEAD:
            return "HEAD";
        case Method::CONNECT:
            return "CONNECT";
        case Method::TRACE:
            return "TRACE";
        case Method::OPTIONS:
            return "OPTIONS";
    }
}

std::string getDefaultMessage(HttpCode code)
{
    switch (code)
    {

    default:
        return "Unknown";

    case HttpCode::Continue:
        return "Continue";

    case HttpCode::Ok:
        return "Ok";
    case HttpCode::Created:
        return "Created";
    case HttpCode::Accepted:
        return "Accepted";
    case HttpCode::NoContent:
        return "NoContent";

    case HttpCode::MultipleChoice:
        return "MultipleChoice";
    case HttpCode::MovedPermentaly:
        return "MovedPermentaly";
    case HttpCode::MovedTemporarily:
        return "MovedTemporarily";
    case HttpCode::NotModified:
        return "NotModified";

    case HttpCode::BadRequest:
        return "BadRequest";
    case HttpCode::Unauthorized:
        return "Unauthorized";
    case HttpCode::Forbidden:
        return "Forbidden";
    case HttpCode::NotFound:
        return "NotFound";

    case HttpCode::InternalServerError:
        return "InternalServerError";
    case HttpCode::NotImplemented:
        return "NotImplemented";
    case HttpCode::BadGateway:
        return "BadGateway";
    case HttpCode::ServiceUnavailable:
        return "ServiceUnavailable";
    case HttpCode::HttpVersionNotSupported:
        return "HttpVersionNotSupported";
    }
}

} // namespace HTTP
} // namespace HTTPP
