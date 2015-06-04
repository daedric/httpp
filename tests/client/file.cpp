/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>

#include "httpp/HttpClient.hpp"

using namespace HTTPP;

BOOST_AUTO_TEST_CASE(file)
{
    HttpClient::Request request;
    request
        .url("file://README.md");

    HttpClient client;
    auto response = client.get(std::move(request));
}
