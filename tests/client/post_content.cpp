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
#include "httpp/http/RestDispatcher.hpp"

using namespace HTTPP;

using HTTPP::HTTP::Request;
using HTTPP::HTTP::Response;
using HTTPP::HTTP::Connection;

static const std::string EXPECTED_BODY(1024 * 100, 'a'); // 1MB

BOOST_AUTO_TEST_CASE(post_content)
{
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::trace);
    commonpp::core::enable_console_logging();

    HttpServer server;
    server.start();

    HTTP::RestDispatcher dispatcher(server);
    dispatcher.add<HTTP::Method::POST>("/", [](HTTP::helper::ReadWholeRequest::Handle
                                                   hndl) {

        if (hndl->body.empty())
        {
            hndl->connection->response()
                .setCode(HTTP::HttpCode::BadRequest)
                .setBody("Expected body!");
            hndl->connection->sendResponse();
            return;
        }

        BOOST_CHECK_EQUAL(std::string(hndl->body.data(), hndl->body.size()),
                          EXPECTED_BODY);

        hndl->connection->response()
            .setCode(HTTP::HttpCode::Ok)
            .connectionShouldBeClosed(true)
            .setBody(EXPECTED_BODY);
        hndl->connection->sendResponse();
    });

    server.bind("localhost", "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .url("http://localhost:8080")
        .addHeader("Expect", "")
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
    commonpp::core::init_logging();
    commonpp::core::set_logging_level(commonpp::trace);
    commonpp::core::enable_console_logging();

    HttpServer server;
    server.start();

    HTTP::RestDispatcher dispatcher(server);
    dispatcher.add<HTTP::Method::POST>("/", [](HTTP::helper::ReadWholeRequest::Handle
                                                   hndl) {

        if (hndl->body.empty())
        {
            hndl->connection->response()
                .setCode(HTTP::HttpCode::BadRequest)
                .setBody("Expected body!");
            hndl->connection->sendResponse();
            return;
        }

        BOOST_CHECK_EQUAL(std::string(hndl->body.data(), hndl->body.size()),
                          EXPECTED_BODY);

        hndl->connection->response()
            .setCode(HTTP::HttpCode::Ok)
            .connectionShouldBeClosed(true)
            .setBody(EXPECTED_BODY);
        hndl->connection->sendResponse();
    });

    server.bind("localhost", {"", "", "", cert, key, ""}, "8080");

    HttpClient client;

    HttpClient::Request request;
    request
        .allowInsecure()
        .url("https://localhost:8080")
        .addHeader("Expect", "")
        .setContent(EXPECTED_BODY);

    auto resp = client.post(std::move(request));
    std::string str(resp.body.data(), resp.body.size());

    BOOST_CHECK(resp.code == HTTP::HttpCode::Ok);
    BOOST_CHECK_EQUAL(EXPECTED_BODY, str);
}
