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
# define _HTTPP_HTPP_REQUEST_HPP_

# include <string>
# include <vector>
# include <iosfwd>
# include <chrono>

# include <httpp/detail/config.hpp>
# include "Protocol.hpp"
# include "httpp/utils/SortedVectorKP.hpp"

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

    std::string Request:getUrl()
    void setDate();
	void clear();

    TimePoint received = Clock::now();
    Method method;

# if HTTPP_PARSER_BACKEND_IS_RAGEL
    boost::string_ref uri;
# else
    std::string uri;
#endif

    int major = 0;
    int minor = 0;

    using QueryParamRef = HTTP::QueryParamRef;

    std::vector<QueryParamRef> query_params;

    auto getSortedQueryParams() const -> decltype(UTILS::create_sorted_vector(query_params))
    {
        return UTILS::create_sorted_vector(query_params);
    }

    std::vector<HeaderRef> headers;

    auto getSortedHeaders() const -> decltype(UTILS::create_sorted_vector(headers))
    {
        return UTILS::create_sorted_vector(headers);
    }

};

std::ostream& operator<<(std::ostream& os, const Request& request);

} // namespace HTTP
} // namespace HTTPP

#endif // !_HTTPP_HTPP_REQUEST_HPP_
