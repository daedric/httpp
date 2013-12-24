/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/HttpCode.hpp"

std::string HTTPP::HTTP::getDefaultMessage(HttpCode code)
{
    switch (code)
    {

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
