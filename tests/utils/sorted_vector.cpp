/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2017 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/SortedVectorKP.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace HTTPP::UTILS;
using namespace HTTPP::HTTP;

BOOST_AUTO_TEST_CASE(case_sensitive_lookup)
{
    SortedVectorKP<std::string_view, std::string_view> kv(
        {{"Content-Length", "15543"},
         {"Content-Type", "text/css"},
         {"Date", "Wed, 05 Apr 2017 16:37:38 GMT"},
         {"last-modified", "Tue, 07 Mar 2017 16:30:34 GMT"},
         {"Server", "MochiWeb/1.0 (Any of you quaids got a smint?)"}});

    BOOST_REQUIRE_EQUAL("", kv["aaa"]); // check for something that would be at
                                        // the beginning of the sorted vec
    BOOST_REQUIRE_EQUAL("15543", kv["Content-Length"]);
    BOOST_REQUIRE_EQUAL("", kv["Nothing"]);
    BOOST_REQUIRE_EQUAL("", kv["Last-Modified"]);
    BOOST_REQUIRE_EQUAL("", kv["ZZZ"]); // and at the end.
}

BOOST_AUTO_TEST_CASE(case_insensitive_lookup)
{
    SortedVectorKP<std::string_view, std::string_view, case_insensitive> kv(
        {{"Content-Length", "15543"},
         {"Content-Type", "text/css"},
         {"Date", "Wed, 05 Apr 2017 16:37:38 GMT"},
         {"last-modified", "Tue, 07 Mar 2017 16:30:34 GMT"},
         {"Server", "MochiWeb/1.0 (Any of you quaids got a smint?)"}});

    BOOST_REQUIRE_EQUAL("", kv["aaa"]); // check for something that would be at
                                        // the beginning of the sorted vec
    BOOST_REQUIRE_EQUAL("15543", kv["Content-Length"]);
    BOOST_REQUIRE_EQUAL("", kv["Nothing"]);
    BOOST_REQUIRE_EQUAL("Tue, 07 Mar 2017 16:30:34 GMT", kv["Last-Modified"]);
    BOOST_REQUIRE_EQUAL("MochiWeb/1.0 (Any of you quaids got a smint?)",
                        kv["Server"]);
    BOOST_REQUIRE_EQUAL("", kv["ZZZ"]); // and at the end.
}

#if HTTPP_RAGEL_BACKEND
BOOST_AUTO_TEST_CASE(case_insensitive_lookup_from_request)
{
    const char request[] = "GET / HTTP/1.1\r\n"
                           "Host: www.google.com\r\n"
                           "user-Agent: curl/7.47.0\r\n"
                           "accept: text/html\r\n"
                           "\r\n\r\n";

    Request req;
    size_t consumed{0};
    bool parsed_ok =
        Parser::parse(request, request + sizeof(request), consumed, req);

    BOOST_CHECK(parsed_ok);
    auto headers = req.getSortedHeaders<case_insensitive>();
    BOOST_REQUIRE_EQUAL("www.google.com", headers["host"]);
    BOOST_REQUIRE_EQUAL("www.google.com", headers["Host"]);
    BOOST_REQUIRE_EQUAL("text/html", headers["accept"]);
    BOOST_REQUIRE_EQUAL("text/html", headers["Accept"]);

    auto normal_headers = req.getSortedHeaders();
    BOOST_REQUIRE_EQUAL("", normal_headers["host"]);
    BOOST_REQUIRE_EQUAL("www.google.com", normal_headers["Host"]);
    BOOST_REQUIRE_EQUAL("text/html", normal_headers["accept"]);
    BOOST_REQUIRE_EQUAL("", normal_headers["Accept"]);
}
#endif
