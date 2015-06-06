/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <cstring>

#include <vector>
#include <string>
#include <algorithm>

#include "httpp/http/client/Response.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace client
{

void parseCurlResponseHeader(const std::vector<char>& headers, Response& response)
{

    auto it = std::begin(headers);
    auto end = std::end(headers);

    if (it == end)
    {
        // In case of a file://xxxx we don't have any header.
        return ;
    }

    it = std::find(it, end, '\r') + 2;

    while (true)
    {
        std::string key;
        std::string value;

        auto key_end = std::find_if(it,
                                    end,
                                    [](const char c)
                                    { return strchr(": \r", c) != nullptr; });

        key.assign(it, key_end);

        it = key_end;

        if (key.empty())
        {
            break;
        }

        while (::isspace(*it) && it != end)
        {
            ++it;
        }

        if (*it != ':')
        {
            throw std::runtime_error("Invalid header, : expected");
        }

        ++it;

        while (*it == ' ' && it != end)
        {
            ++it;
        }

        auto value_end = std::find(it, end, '\r');
        if (value_end != it)
        {
            value.assign(it, value_end);
            it = value_end;
        }

        response.headers.emplace_back(std::move(key), std::move(value));

        if (!(*it == '\r' && *(it + 1) == '\n'))
        {
            throw std::runtime_error("Invalid Header, expect \\r\\n at "
                                     "the end of an header");
        }
        ++it;
        ++it;
    }
}

} // namespace client
} // namespace HTTP
} // namespace HTTPP
