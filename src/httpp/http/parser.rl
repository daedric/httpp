#include <iostream>
#include <vector>
#include <string>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/URL.hpp"

%%{
    machine http;
    write data;
}%%

%%{
action start_path {
    path_begin = fpc;
}

action end_path {
    path_end = fpc;
    request.uri.assign(path_begin, path_end);
    path_begin = path_end = nullptr;
}

action start_key {
    k_begin = fpc;
}

action end_key {
    k_end = fpc;
    request.headers.emplace_back(std::make_pair<std::string>({k_begin, k_end}, ""));
    k_begin = k_end = nullptr;
}

action start_value {
    v_begin = fpc;
}

action end_value {
    v_end = fpc;
    request.headers.back().second = {v_begin, v_end};
    v_begin = v_end = nullptr;
}

action start_qkey {
    k_begin = fpc;
}

action end_qkey {
    k_end = fpc;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::url_decode(k_begin, k_end)}, ""));
    k_begin = k_end = nullptr;
}

action start_qvalue {
    v_begin = fpc;
}

action end_qvalue {
    v_end = fpc;
    request.query_params.back().second = UTILS::url_decode(v_begin, v_end);
    v_begin = v_end = nullptr;
}

action start_method {
    method_begin = fpc;
}

action end_method {
    try
    {
        request.method = method_from(method_begin);
    }
    catch(...)
    {
        fbreak;
    }
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

    const char *k_begin, *k_end;
    const char *v_begin, *v_end;
    const char *path_begin, *path_end;
    const char *method_begin;

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

