/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include <sstream>

#include <boost/test/unit_test.hpp>

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/VectorStreamBuf.hpp"

using HTTPP::HTTP::Parser;
using HTTPP::HTTP::Request;
using HTTPP::UTILS::VectorStreamBuf;

namespace std
{
ostream& operator<<(ostream& os, const HTTPP::HTTP::Header& h)
{
    return os << h.first << ": " << h.second;
}
} // namespace std

#if HTTPP_PARSER_BACKEND_IS_STREAM
BOOST_AUTO_TEST_CASE(parser_streambuf)
{
    const std::string query =
        "GET /test?1234=4321 HTTP/1.1\r\n"
        "Test: coucou\r\n"
        "Salut : Toi\r\n"
        "Content-Type:\r\n"
        "Hello:world!\r\n"
        "Incomplete2:\r\n"
        "Test2:Test3\r\n"
        "\r\nqwertyuiop";

    std::vector<char> vect(std::begin(query), std::end(query));
    VectorStreamBuf buf(vect);
    std::istream is(&buf);

    Request request;
    bool b = Parser::parse(is, request);
    BOOST_CHECK(b);

    buf.shrinkVector();
    std::string remaining(vect.data(), vect.size());
    BOOST_CHECK_EQUAL(remaining, "qwertyuiop");

    BOOST_CHECK_EQUAL(request.headers[0], HTTPP::HTTP::Header("Test", "coucou"));
    BOOST_CHECK_EQUAL(request.headers[1], HTTPP::HTTP::Header("Salut", "Toi"));
    BOOST_CHECK_EQUAL(request.headers[2], HTTPP::HTTP::Header("Content-Type", ""));
    BOOST_CHECK_EQUAL(request.headers[3], HTTPP::HTTP::Header("Hello", "world!"));
    BOOST_CHECK_EQUAL(request.headers[4], HTTPP::HTTP::Header("Incomplete2", ""));
    BOOST_CHECK_EQUAL(request.headers[5], HTTPP::HTTP::Header("Test2", "Test3"));
}
#else
BOOST_AUTO_TEST_CASE(parser_ragel)
{
    const std::string query =
        "POST /test?1234=4321 HTTP/1.1\r\n"
        "Test: coucou\r\n"
        "Salut : Toi\r\n"
        "Content-Type:\r\n"
        "Hello:world!\r\n"
        "Incomplete2:\r\n"
        "Test2:Test3\r\n"
        "\r\nqwertyuiop";

    std::vector<char> vect(std::begin(query), std::end(query));

    Request request;
    size_t consumed = 0;
    bool b = Parser::parse(query.data(), query.data() + query.size(), consumed, request);
    BOOST_REQUIRE(b);
    vect.erase(vect.begin(), vect.begin() + consumed);

    std::string remaining(vect.data(), vect.size());
    BOOST_CHECK_EQUAL(remaining, "qwertyuiop");

    BOOST_CHECK(request.headers[0] == HTTPP::HTTP::HeaderRef("Test", "coucou"));
    BOOST_CHECK(request.headers[1] == HTTPP::HTTP::HeaderRef("Salut", "Toi"));
    BOOST_CHECK(request.headers[2] == HTTPP::HTTP::HeaderRef("Content-Type", ""));
    BOOST_CHECK(request.headers[3] == HTTPP::HTTP::HeaderRef("Hello", "world!"));
    BOOST_CHECK(request.headers[4] == HTTPP::HTTP::HeaderRef("Incomplete2", ""));
    BOOST_CHECK(request.headers[5] == HTTPP::HTTP::HeaderRef("Test2", "Test3"));
}
#endif
