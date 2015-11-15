/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Utils.hpp"

namespace HTTPP
{
namespace HTTP
{

static const std::string CONNECTION = "Connection";
static const std::string CLOSE = "Close";
static const std::string KEEPALIVE = "Keep-Alive";

static inline bool is_iequal(const char* s1, size_t n1, const char* s2, size_t n2)
{
    return n1 == n2 && ::strncasecmp(s1, s2, n1) == 0;
}

#define CMP(a, b) is_iequal(a.data(), a.size(), b.data(), b.size())

void setShouldConnectionBeClosed(const Request& request, Response& response)
{
    const auto& headers = request.headers;
    for (const auto& h : headers)
    {
        if (CMP(h.first, CONNECTION))
        {
            const auto& connection = h.second;
            if (CMP(connection, CLOSE))
            {
                response.connectionShouldBeClosed(true);
                return;
            }

            if (CMP(connection, KEEPALIVE))
            {
                response.connectionShouldBeClosed(false);
                response.addHeader(CONNECTION, KEEPALIVE);
                return;
            }

            break;
        }
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
