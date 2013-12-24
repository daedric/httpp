/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
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

using H = Request::Header;

namespace std
{
ostream& operator<<(ostream& os, const H& h)
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

    BOOST_CHECK_EQUAL(request.headers[0], H("Test", "coucou"));
    BOOST_CHECK_EQUAL(request.headers[1], H("Salut", "Toi"));
    BOOST_CHECK_EQUAL(request.headers[2], H("Content-Type", ""));
    BOOST_CHECK_EQUAL(request.headers[3], H("Hello", "world!"));
    BOOST_CHECK_EQUAL(request.headers[4], H("Incomplete2", ""));
    BOOST_CHECK_EQUAL(request.headers[5], H("Test2", "Test3"));
}


