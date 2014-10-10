/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <iostream>

namespace HTTPP
{
namespace HTTP
{

static const std::string CLOSE = "Close";
static const std::string KEEPALIVE = "Keep-Alive";

void setShouldConnectionBeClosed(const Request& request, Response& response)
{
    auto headers = request.getSortedHeaders();
    auto const& connection = headers["Connection"];

    if (boost::iequals(connection, CLOSE))
    {
        response.connectionShouldBeClosed(true);
        return;
    }

    if (boost::iequals(connection, KEEPALIVE))
    {
        response.connectionShouldBeClosed(false);
        response.addHeader("Connection", KEEPALIVE);
        return;
    }

    if (request.major == 1 && request.minor == 1)
    {
        response.connectionShouldBeClosed(false);
        return;
    }

    response.connectionShouldBeClosed(true);
    return;
}

} // namespace HTTP
} // namespace HTTPP
