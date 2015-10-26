/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Parser.hpp"

#include <cstring>

#include <istream>
#include <iterator>
#include <functional>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/http/Request.hpp"
#include "httpp/utils/URL.hpp"

namespace HTTPP
{
namespace HTTP
{

#if HTTPP_PARSER_BACKEND == HTTPP_STREAM_BACKEND

CREATE_LOGGER(parser_logger, "httpp::HttpServer::Parser");

using Iterator = std::istream_iterator<char>;
static const Iterator EOS = Iterator();

static const char* HTTP_DELIMITER = "\r\n";

static bool match(Iterator& it, const char c)
{
    if (it != EOS && *it == c)
    {
        ++it;
        return true;
    }
    return false;
}

static bool expect(Iterator& it, const std::string& expected)
{
    for (auto eit = std::begin(expected), eend = std::end(expected); eit != eend;
         ++eit)
    {
        if (*eit != *it)
        {
            LOG(parser_logger, error)
                << "Error happened parsing request: expected: " << expected
                << " got: " << *it << " (" << std::hex << (int)*it << ")";
            return false;
        }

        ++it;
    }

    return true;
}

static bool parse_method(Iterator& it, Request& request)
{
    switch (*it)
    {
        case 'H':
            if (expect(it, "HEAD"))
            {
                request.method = Method::HEAD;
                return true;
            }
            break;
        case 'G':
            if (expect(it, "GET"))
            {
                request.method = Method::GET;
                return true;
            }
            break;

        case 'P':
            ++it;
            if (*it == 'O')
            {
                if (expect(it, "OST"))
                {
                    request.method = Method::POST;
                    return true;
                }
            }
            else if (*it == 'U')
            {
                if (expect(it, "UT"))
                {
                    request.method = Method::PUT;
                    return true;
                }
            }
            break;
        case 'D':
            if (expect(it, "DELETE"))
            {
                request.method = Method::DELETE_;
                return true;
            }
            break;
        case 'O':
            if (expect(it, "OPTIONS"))
            {
                request.method = Method::OPTIONS;
                return true;
            }
            break;
        case 'T':
            if (expect(it, "TRACE"))
            {
                request.method = Method::TRACE;
                return true;
            }
            break;

        case 'C':
            if (expect(it, "CONNECT"))
            {
                request.method = Method::CONNECT;
                return true;
            }
            break;
    }

    return false;
}

static void skipws(Iterator& it)
{
    while (it != EOS && ::isspace(*it))
    {
        ++it;
    }
}

static bool consumeUntil(Iterator& it, std::string& acc, const char* charset)
{
    while (it != EOS)
    {
        char c = *it;
        if (::strchr(charset, c))
        {
            return true;
        }

        acc += c;
        ++it;
    }

    return false;
}

static bool parse_uri(Iterator& it, Request& request)
{

    skipws(it);

    consumeUntil(it, request.uri, "? ");
    if (match(it, '?'))
    {
        do
        {
            std::string key;
            std::string value;

            bool res = consumeUntil(it, key, "&= ");
            if (match(it, '=')) {
                res = res && consumeUntil(it, value, "& ");
                key = UTILS::url_decode(key);
                value = UTILS::url_decode(value);
                request.query_params.emplace_back(std::move(key), std::move(value));
            }
            else
            {
                request.query_params.emplace_back(std::move(key), "");
            }

            if (!res)
            {
                return res;
            }
        } while (match(it, '&'));
    }

    return it != EOS;
}

static bool parse_http_version(Iterator& it, Request& request)
{
    skipws(it);
    bool res = expect(it, "HTTP/");
    std::string major, minor;
    res = res && consumeUntil(it, major, ".");
    res = res && expect(it, ".");
    res = res && consumeUntil(it, minor, "\r");

    if (res)
    {
        try
        {
            request.major = std::stoi(major);
            request.minor = std::stoi(minor);
        }
        catch (std::exception const& ex)
        {
            LOG(parser_logger, error) << "Cannot parse Major/Minor: "
                                      << ex.what();
            return false;
        }
    }

    return res;
}

static bool parse_request_line(Iterator& it, Request& request)
{
    bool ret = true;
    ret = ret && parse_method(it, request);
    ret = ret && parse_uri(it, request);
    ret = ret && parse_http_version(it, request);
    ret = ret && expect(it, HTTP_DELIMITER);
    return ret;
}

static bool parse_headers(Iterator& it, Request& request)
{
    bool res = true;
    while (res)
    {
        std::string key;
        std::string value;

        res = consumeUntil(it, key, ": \r");
        if (key.empty())
        {
            return res;
        }

        skipws(it);
        res = res && expect(it, ":");

        while (match(it, ' '))
        {
        }

        res = res && consumeUntil(it, value, "\r");

        if (res)
        {
            request.headers.emplace_back(std::move(key), std::move(value));
        }

        res = res && expect(it, HTTP_DELIMITER);
    }

    return false;
}

bool Parser::parse(std::istream& is, Request& request)
{
    std::noskipws(is);
    Iterator it(is);

    bool ret = true;
    ret = ret && parse_request_line(it, request);
    ret = ret && parse_headers(it, request);
    ret = ret && match(it, '\r');
    ret = ret && *it == '\n';

    return ret;
}

#endif // if HTTPP_PARSER_BACKEND == HTTPP_STREAM_BACKEND

bool Parser::isComplete(const char* buffer, size_t n)
{
    if (n < 4)
    {
        return false;
    }

    static char const MARKER[] = { '\r', '\n', '\r', '\n' };

    auto buffer_end = buffer + n;
    return std::search(
            buffer,
            buffer_end,
            std::begin(MARKER),
            std::end(MARKER)) != buffer_end;

}

} // namespace HTTP
} // namespace HTTPP
