/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_REQUEST_HPP_
#define _HTTPP_HTPP_REQUEST_HPP_

#include <chrono>
#include <iosfwd>
#include <string>
#include <vector>

#include "Protocol.hpp"
#include "httpp/utils/SortedVectorKP.hpp"
#include <httpp/detail/config.hpp>

namespace HTTPP
{
namespace HTTP
{

struct Request
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    Request()
    {
        query_params.reserve(10);
        headers.reserve(10);
    }

    void setDate();
    void clear();

    TimePoint received = Clock::now();
    Method method;

#if HTTPP_PARSER_BACKEND_IS_RAGEL
    std::string_view uri;
#else
    std::string uri;
#endif

    int major = 0;
    int minor = 0;

    using QueryParamRef = HTTP::QueryParamRef;

    std::vector<QueryParamRef> query_params;

    template <typename Comparator = std::less<QueryParamRef::first_type>>
    auto getSortedQueryParams() const
    {
        return UTILS::create_sorted_vector<QueryParamRef::first_type, QueryParamRef::second_type, Comparator>(query_params
        );
    }

    std::vector<HeaderRef> headers;

    template <typename Comparator = std::less<HeaderRef::first_type>>
    auto getSortedHeaders() const
    {
        return UTILS::create_sorted_vector<HeaderRef::first_type, HeaderRef::second_type, Comparator>(headers
        );
    }
};

std::ostream& operator<<(std::ostream& os, const Request& request);

} // namespace HTTP
} // namespace HTTPP

#endif // !_HTTPP_HTPP_REQUEST_HPP_
