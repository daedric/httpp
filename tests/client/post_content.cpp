/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include <boost/test/unit_test.hpp>

#include "httpp/HttpServer.hpp"
#include "httpp/HttpClient.hpp"
#include "httpp/utils/Exception.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static const std::string EXPECTED_BODY = R"*(PREFIX ?= /usr/local

all:
	make -C build all

clean:
	make -C build clean

cmake:
	rm -rf build
	mkdir build
	cd build && cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

package:
	make -C build package

test:
	make -C build test

re : cmake all)*";

static Connection* gconnection = nullptr;
void body_handler(const boost::system::error_code& ec, const char* buffer, size_t n)
{
    static std::string body_read;

    if (ec == boost::asio::error::eof)
    {
        BOOST_CHECK_EQUAL(body_read, EXPECTED_BODY);
        body_read.clear();
        (gconnection->response() = Response(HTTP::HttpCode::Ok))
            .setBody(EXPECTED_BODY)
            .connectionShouldBeClosed(true);
        gconnection->sendResponse();
    }
    else if (ec)
    {
        throw HTTPP::UTILS::convert_boost_ec_to_std_ec(ec);
    }
    else
    {
        body_read.append(buffer, n);
    }
}

void handler(Connection* connection, Request&& request)
{
    gconnection = connection;
    auto headers = request.getSortedHeaders();
    auto content_length = headers["Content-Length"];
    if (!content_length.empty())
    {
        auto size = std::stoi(to_string(headers["Content-Length"]));
        connection->readBody(size, &body_handler);
    } else {
        connection->response()
            .setCode(HTTP::HttpCode::BadRequest)
            .setBody("Expected body!");
        connection->sendResponse();
    }
}

BOOST_AUTO_TEST_CASE(post_content)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .setContent(EXPECTED_BODY);

    auto resp = client.post(std::move(request));
    std::string str(resp.body.data(), resp.body.size());

    BOOST_CHECK_EQUAL(EXPECTED_BODY, str);
}

static const std::string cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICATCCAWoCCQCW36Y5DLTefjANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJY\n"
    "WDETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"
    "cyBQdHkgTHRkMB4XDTE0MTIxMzEyMzQ1MloXDTE1MDExMjEyMzQ1MlowRTELMAkG\n"
    "A1UEBhMCWFgxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"
    "IFdpZGdpdHMgUHR5IEx0ZDCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA52Tu\n"
    "oj/NxKY1XbL2rqmUhCnrW6Ymwi7fX1qj6xL3pWnrpv9Wfi0CVvGJOg/ngsbzoM9z\n"
    "UI8hUpRjJ8sm+ZT7HA46u+9f70w+apwtF+7qrmgeKIX5KpW84V9wpzbdvxzkEG2w\n"
    "RvUWZQhG5yVbqkWR3ozt8pCCuriBduRPo0GW1j0CAwEAATANBgkqhkiG9w0BAQsF\n"
    "AAOBgQCwx0o/WAitPJex7Qm16ZFWCy2rMV23gfJhmjLjg2hh7LkU4Mzynq5N8CMZ\n"
    "y8f2erPd5jzY61CKF96CDHinQcJh4EEmV0WLPQrgZr8oHCtQsUpZNoYcSTQI2P7H\n"
    "Mz/KSycVdAJ8E0Lh8RYvGYlo3sUc/KCd/oL8dvbVR5V4Uhkwgg==\n"
    "-----END CERTIFICATE-----";

static const std::string key =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXAIBAAKBgQDnZO6iP83EpjVdsvauqZSEKetbpibCLt9fWqPrEvelaeum/1Z+\n"
    "LQJW8Yk6D+eCxvOgz3NQjyFSlGMnyyb5lPscDjq771/vTD5qnC0X7uquaB4ohfkq\n"
    "lbzhX3CnNt2/HOQQbbBG9RZlCEbnJVuqRZHejO3ykIK6uIF25E+jQZbWPQIDAQAB\n"
    "AoGBALziVi2ZeaVxq5Rd9yHiibpM4bOBcJgIXQxgV/gXFpIvYU6Dlh4PLZ69MbB0\n"
    "tR26u7rkPJRhEelv+XTNT3ZiV4skqGmMhTqO8RNv6wpHNMeTkCTLCvA1wuMjdH7M\n"
    "2tEFPctuAO0L6WieIVXx2gFe3FS51RAGmUIGft3s/L05SiUBAkEA/PdEsGcHazRw\n"
    "6OBNsZZU+t2aKkDSInKgbsdZP47JTovpCn7fnhyG1QaLSHUNvVHSNz6zLUhHUCRn\n"
    "ccGsiK6UfQJBAOorbZ0MQJ81igGiHuySfhIxmat84I99TJJQ2gB4KFV6Ta43J8b1\n"
    "tjdgZqSwHrje07Ctp596lxaRkocL/5WltMECQDMEJy0wShW7yL2eZuzWtaK2SF4X\n"
    "Honl9icBOyWPRVf67W+5cJ2xgRu7KyHcwX2Z37xWf8o1FnS6MsaFWadjFf0CQEFY\n"
    "PQ5GDQJgflJWWyrI9kU9chRYlJF19Zge17ap0ReJOYQUABWVG3P4gqwSOYB9LbVv\n"
    "wAbQvridhrYIsxmfVAECQFW2a3m2OmI22rUz4JlGMd/gOCPHTcXgAy8byxxpVDB8\n"
    "sEsylFILPN5doJ+4A2B6bM02/eum3eKcd6cUDnSoeqc=\n"
    "-----END RSA PRIVATE KEY-----";

BOOST_AUTO_TEST_CASE(https_post_content)
{
    HttpServer server;
    server.start();
    server.setSink(&handler);
    server.bind("localhost", {"", "", "", cert, key, ""}, "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .allowInsecure()
        .url("https://localhost:8080")
        .setContent(EXPECTED_BODY);

    auto resp = client.post(std::move(request));
    std::string str(resp.body.data(), resp.body.size());

    BOOST_CHECK_EQUAL(EXPECTED_BODY, str);
}
