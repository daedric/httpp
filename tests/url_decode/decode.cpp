/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>

#include "httpp/utils/URL.hpp"

using namespace HTTPP::UTILS;

BOOST_AUTO_TEST_CASE(basic_decode)
{
    const std::string ENCODED_URL =
        "Type+or+paste+in+the+text+you+want+to+HTML+encode%2c%0d%0a+then+press+"
        "the+%e2%80%98Encode%e2%80%99+button%2c+or+read+a+brief+%0d%"
        "0aexplanation+of+the+process+of+HTML+encoding.%0d%0a%0d%0a";

    const std::string ENCODED_URL_PCT =
        "Type%20or%20paste%20in%20the%20text%20you%20want%20to%20HTML%20encode%"
        "2c%0d%0a%20then%20press%20the%20%e2%80%98Encode%e2%80%99%20button%2c%"
        "20or%20read%20a%20brief%20%0d%0aexplanation%20of%20the%20process%20of%"
        "20HTML%20encoding.%0d%0a%0d%0a";

    const std::string DECODED_URL =
        "Type or paste in the text you want to "
        "HTML encode,\r\n then press the ‘Encode’ "
        "button, or read a brief \r\nexplanation of "
        "the process of HTML encoding.\r\n\r\n";

    BOOST_CHECK_EQUAL(url_decode(ENCODED_URL), DECODED_URL);
    BOOST_CHECK_EQUAL(url_encode(DECODED_URL), ENCODED_URL_PCT);
}
