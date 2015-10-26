/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include <httpp/detail/config.hpp>

#include "httpp/http/Parser.hpp"

#include <sstream>
#include "httpp/utils/VectorStreamBuf.hpp"
#include <boost/test/unit_test.hpp>
#include "httpp/http/Request.hpp"

#if HTTPP_PARSER_BACKEND == HTTPP_STREAM_BACKEND

using HTTPP::UTILS::VectorStreamBuf;
using HTTPP::HTTP::Request;
using HTTPP::HTTP::Parser;

BOOST_AUTO_TEST_CASE(parser_streambuf)
{
    const std::string query = "GET /test?1234=4321 HTTP/1.1\r\n\r\nqwertyuiop";

    std::vector<char> vect(std::begin(query), std::end(query));
    VectorStreamBuf buf(vect);
    std::istream is(&buf);

    Request request;
    bool b = Parser::parse(is, request);
    BOOST_CHECK(b);

    buf.shrinkVector();
    std::string remaining(vect.data(), vect.size());
    BOOST_CHECK_EQUAL(remaining, "qwertyuiop");
}

#else

BOOST_AUTO_TEST_CASE(skip)
{}

#endif
