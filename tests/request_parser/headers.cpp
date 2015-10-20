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

#include <sstream>
#include "httpp/utils/VectorStreamBuf.hpp"
#include <boost/test/unit_test.hpp>
#include "httpp/http/Request.hpp"

using HTTPP::UTILS::VectorStreamBuf;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Parser;

namespace std
{
ostream& operator<<(ostream& os, const HTTPP::HTTP::Header& h)
{
    return os << h.first << ": " << h.second;
}
}

BOOST_AUTO_TEST_CASE(parser_streambuf)
{
    const std::string query = "GET /test?1234=4321 HTTP/1.1\r\n"
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

BOOST_AUTO_TEST_CASE(parser_ragel)
{
    const std::string query = "POST /test?1234=4321 HTTP/1.1\r\n"
        "Test: coucou\r\n"
        "Salut : Toi\r\n"
        "Content-Type:\r\n"
        "Hello:world!\r\n"
        "Incomplete2:\r\n"
        "Test2:Test3\r\n"
        "\r\nqwertyuiop";

    std::vector<char> vect(std::begin(query), std::end(query));

    Request request;
    size_t consumed;
    bool b = Parser::parse(query.data(), query.data() + query.size(), consumed, request);
    BOOST_CHECK(b);
    vect.erase(vect.begin(), vect.begin() + consumed);

    std::string remaining(vect.data(), vect.size());
    BOOST_CHECK_EQUAL(remaining, "qwertyuiop");

    BOOST_CHECK_EQUAL(request.headers[0], HTTPP::HTTP::Header("Test", "coucou"));
    BOOST_CHECK_EQUAL(request.headers[1], HTTPP::HTTP::Header("Salut", "Toi"));
    BOOST_CHECK_EQUAL(request.headers[2], HTTPP::HTTP::Header("Content-Type", ""));
    BOOST_CHECK_EQUAL(request.headers[3], HTTPP::HTTP::Header("Hello", "world!"));
    BOOST_CHECK_EQUAL(request.headers[4], HTTPP::HTTP::Header("Incomplete2", ""));
    BOOST_CHECK_EQUAL(request.headers[5], HTTPP::HTTP::Header("Test2", "Test3"));
}
