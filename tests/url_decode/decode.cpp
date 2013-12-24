/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/utils/URL.hpp"
#include <boost/test/unit_test.hpp>

using namespace HTTPP::UTILS;

BOOST_AUTO_TEST_CASE(basic_decode)
{
    const std::string ENCODED_URL =
        "Type+or+paste+in+the+text+you+want+to+HTML+encode%2c%0d%0a+then+press+"
        "the+%e2%80%98Encode%e2%80%99+button%2c+or+read+a+brief+%0d%"
        "0aexplanation+of+the+process+of+HTML+encoding.%0d%0a%0d%0a";

    const std::string DECODED_URL = "Type or paste in the text you want to "
                                    "HTML encode,\r\n then press the ‘Encode’ "
                                    "button, or read a brief \r\nexplanation of "
                                    "the process of HTML encoding.\r\n\r\n";

    BOOST_CHECK_EQUAL(url_decode(ENCODED_URL), DECODED_URL);
}
