/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */

#include <vector>
#include <string>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/URL.hpp"
#include "httpp/utils/LazyDecodedValue.hpp"

#if HTTPP_PARSER_BACKEND == HTTPP_RAGEL_BACKEND

#define TOKEN_LEN size_t(token_end - token_begin)
#define TOKEN_REF boost::string_ref(token_begin, TOKEN_LEN)

%%{
    machine http;
    write data;
}%%

%%{
action start_path {
    token_begin = fpc;
}

action end_path {
    token_end = fpc;
    request.uri = TOKEN_REF;
    token_begin = token_end = nullptr;
}

action start_key {
    token_begin = fpc;
}

action end_key {
    token_end = fpc;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}

action start_value {
    token_begin = fpc;
}

action end_value {
    token_end = fpc;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}

action start_qkey {
    token_begin = fpc;
}

action end_qkey {
    token_end = fpc;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}

action start_qvalue {
    token_begin = fpc;
}

action end_qvalue {
    token_end = fpc;
    request.query_params.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}

action start_method {
    token_begin = fpc;
}

action end_method {
    try
    {
        request.method = method_from(token_begin);
    }
    catch(...)
    {
        fbreak;
    }

    token_begin = nullptr;
}

}%%

namespace HTTPP { namespace HTTP {
bool Parser::parse(const char* start,
                   const char* end,
                   size_t& consumed,
                   Request& request)
{
    const char *p = start;
    int cs;

    const char *token_begin, *token_end;

    %%{
        method = ("GET" | "POST" | "HEAD" | "PUT" | "DELETE" | "OPTIONS" | "TRACE" | "CONNECT");

        identifier = (alnum | '-')+;

        query_identifier = any** -- (space | '&');
        query_variable = (query_identifier -- '=') >start_qkey %end_qkey;
        query_value = query_identifier >start_qvalue %end_qvalue;

        parameter = query_variable ("=" query_value)?;
        path = (any+ -- (space | "?")) >start_path %end_path;
        query = path ("?" parameter ('&' parameter)*)?;

        major = digit >{ request.major = fc - '0';};
        minor = digit >{request.minor = fc - '0';};
        status_line = (method >start_method @end_method) space query space "HTTP/" major "." minor "\r\n";

        key = identifier >start_key %end_key;
        value = (any+ -- "\r\n") >start_value %end_value;
        header = key space* ":" space* value? "\r\n";
        headers = header* "\r\n";
        main := (status_line headers)
                %~{
                    fbreak;
                };

        write init;
        write exec noend;
    }%%

    if (cs < http_first_final)
    {
        GLOG(error) << "Invalid request read, cannot parse: " << std::string(p, end);
        return false;
    }

    consumed = p - start;
    return true;
}

} }
#endif
