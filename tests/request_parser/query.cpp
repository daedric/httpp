/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/detail/config.hpp"
#include "httpp/http/Parser.hpp"

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "httpp/http/Request.hpp"

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Parser;

namespace std
{
ostream& operator<<(ostream& os, const Request::QueryParamRef& h)
{
    return os << h.first << ": " << h.second;
}
}

#if HTTPP_PARSER_BACKEND_IS_STREAM
Request parse(const std::string& req)
{
    const std::string query = "GET " + req + " HTTP/1.1\r\n\r\n";
    std::istringstream is(query);

    Request request;
    bool b = Parser::parse(is, request);
    BOOST_CHECK(b);
    return request;
}
#elif HTTPP_PARSER_BACKEND_IS_RAGEL
Request parse(const std::string& req)
{
    const std::string query = "GET " + req + " HTTP/1.1\r\n\r\n";

    Request request;
    size_t consumed;
    bool b = Parser::parse(query.data(), query.data() + query.size(), consumed,
                           request);
    BOOST_CHECK(b);
    return request;
}
#endif

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_no_query)
{
    parse("/bid?");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_no_var)
{
    parse("/bid?&");
}

void testQueryParam(const Request& request,
                    const std::string& var,
                    const std::string& expectedValue)
{
    for (const auto& p : request.query_params)
    {
        if (p.first == var)
        {
            BOOST_CHECK_EQUAL(p.second, expectedValue);
            return;
        }
    }

    BOOST_CHECK(false);
}

// From Google url
BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g1)
{
    auto request = parse("/?arg1=1&arg2=2&bar");

    testQueryParam(request, "arg1", "1");
    testQueryParam(request, "arg2", "2");
    testQueryParam(request, "bar", "");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g2)
{
    // Empty param at the end.
    auto request = parse("/?foo=bar&");
    testQueryParam(request, "foo", "bar");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g3)
{
    // Empty param at the beginning.
    auto request = parse("/?&foo=bar");
    testQueryParam(request, "", "");
    testQueryParam(request, "foo", "bar");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g4)
{
    // Empty key with value.
    auto request = parse("http://www.google.com?=foo");
    testQueryParam(request, "", "foo");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g5)
{
    // Empty value with key.
    auto request = parse("/?foo=");
    testQueryParam(request, "foo", "");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_g6)
{
    // Empty key and values.
    auto request = parse("/?&&==&=");
    BOOST_CHECK_EQUAL(request.query_params[0].second, "");
    BOOST_CHECK_EQUAL(request.query_params[1].second, "");
    BOOST_CHECK_EQUAL(request.query_params[2].second, "=");
    BOOST_CHECK_EQUAL(request.query_params[3].second, "");
}

BOOST_AUTO_TEST_CASE(test_http_header_query_parser_sorted)
{
    // Empty key and values.
    auto request = parse("/?z=z&y=y&a=a&b=b");
    auto params = request.getSortedQueryParams();
    BOOST_CHECK_EQUAL(params["a"], "a");
    BOOST_CHECK_EQUAL(params["b"], "b");
    BOOST_CHECK_EQUAL(params["y"], "y");
    BOOST_CHECK_EQUAL(params["z"], "z");

    BOOST_CHECK_EQUAL(params["something"], "");

    Request::QueryParamRef a {"a", "a"};
    Request::QueryParamRef b {"b", "b"};
    Request::QueryParamRef y {"y", "y"};
    Request::QueryParamRef z {"z", "z"};

    BOOST_CHECK_EQUAL(params[0], a);
    BOOST_CHECK_EQUAL(params[1], b);
    BOOST_CHECK_EQUAL(params[2], y);
    BOOST_CHECK_EQUAL(params[3], z);
}

BOOST_AUTO_TEST_CASE(empty_query)
{
    // Empty key and values.
    auto request = parse("/");
    auto params = request.getSortedQueryParams();
    BOOST_CHECK_EQUAL(params["a"], "");
    BOOST_CHECK_EQUAL(params["b"], "");
    BOOST_CHECK_EQUAL(params["y"], "");
    BOOST_CHECK_EQUAL(params["z"], "");
}
