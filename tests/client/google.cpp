/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <boost/test/unit_test.hpp>

#include "httpp/HttpClient.hpp"
#include "httpp/HttpServer.hpp"

using namespace HTTPP;

BOOST_AUTO_TEST_CASE(follow_redirect)
{
    HttpClient::Request request;
    request.url("http://www.google.fr/");

    HttpClient client;
    auto response = client.get(std::move(request));
    for (const auto& h : response.headers)
    {
        std::cout << h.first << ": " << h.second << std::endl;
    }

    std::string str(response.body.data(), response.body.size());
}
