/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include <commonpp/core/string/stringify.hpp>
#include <commonpp/core/string/std_tostring.hpp>

#include "httpp/http/Request.hpp"
#include <ostream>

namespace HTTPP
{
namespace HTTP
{
std::ostream& operator<<(std::ostream& os, const Request& request)
{
    os << to_string(request.method) << " ";
    std::string uri = commonpp::string::stringify(request.uri);
    if (!request.query_params.empty())
    {
        uri += '?';
        for (auto const& q : request.query_params)
        {
            uri += q.first + "=" + to_string(q.second) + "&";
        }
    }

    os << uri << " HTTP/" << request.major << "." << request.minor << "\n";
    for (auto const& h : request.headers)
    {
        os << h.first << ": " << h.second << "\n";
    }

    return os;
}

std::string getUrl()
{ 
	return request.uri;
}

void Request::setDate()
{
    received = Clock::now();
}

void Request::clear()
{
    uri.clear();
    headers.clear();
    query_params.clear();
    major = minor = 0;
}

} // namespace HTTP
} // namespace HTTPP

