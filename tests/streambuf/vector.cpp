/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include <istream>
#include <iterator>

#include <boost/test/unit_test.hpp>

#include "httpp/utils/VectorStreamBuf.hpp"

using HTTPP::UTILS::VectorStreamBuf;

BOOST_AUTO_TEST_CASE(vector_stream_buff)
{
    std::vector<char> test;
    const std::string data = "hello world!12345";

    for (auto e : data)
    {
        test.push_back(e);
    }

    VectorStreamBuf buff(test);

    std::istream is(&buff);
    std::noskipws(is);
    std::istream_iterator<char> it(is);

    std::string tmp;
    for (; *it != '!'; ++it)
    {
        tmp += *it;
    }

    BOOST_CHECK_EQUAL(tmp, "hello world");

    buff.shrinkVector();
    std::string remaining(test.data(), test.size());
    BOOST_CHECK_EQUAL(remaining, "12345");
}
