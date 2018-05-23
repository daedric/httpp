/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Heye Voecking.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "config.h"
#include "httpp/HttpClient.hpp"

using namespace HTTPP;

BOOST_AUTO_TEST_CASE(file)
{
    HttpClient::Request request;
    request.url("file://" + PROJECT_ROOT + "/README.md");

    HttpClient client;
    auto response = client.get(std::move(request));

    std::ifstream file(PROJECT_ROOT + "/README.md");
    std::noskipws(file);
    std::istream_iterator<char> it(file), end;
    std::string file_content(it, end);

    BOOST_CHECK_EQUAL(file_content,
                      std::string(response.body.begin(), response.body.end()));
}
