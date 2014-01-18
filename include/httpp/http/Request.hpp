/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
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

# include "httpp/utils/SortedVectorKP.hpp"

namespace HTTPP
{
namespace HTTP
{

struct Request
{

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    enum class Method
    {
        HEAD,
        GET,
        POST,
        PUT,
        DELETE_, // '_' for msvc workaround
        OPTIONS,
        TRACE,
        CONNECT
    };

    const TimePoint received = Clock::now();
    Method method;
    std::string uri;
    int major;
    int minor;

    using QueryParam = std::pair<std::string, std::string>;
    std::vector<QueryParam> query_params;

    auto getSortedQueryParams() const -> decltype(UTILS::create_sorted_vector(query_params))
    {
        return UTILS::create_sorted_vector(query_params);
    }

    using Header = std::pair<std::string, std::string>;
    std::vector<Header> headers;

    auto getSortedHeaders() const -> decltype(UTILS::create_sorted_vector(headers))
    {
        return UTILS::create_sorted_vector(headers);
    }

};

std::ostream& operator<<(std::ostream& os, const Request& request);

} // namespace HTTP
} // namespace HTTPP

#endif // !_HTTPP_HTPP_REQUEST_HPP_
