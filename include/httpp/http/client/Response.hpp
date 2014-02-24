/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__HTTP_CLIENT_RESPONSE_HPP_
# define _HTTPP__HTTP_CLIENT_RESPONSE_HPP_

# include <vector>

# include <httpp/http/Protocol.hpp>
# include <httpp/http/client/Request.hpp>
# include <httpp/utils/SortedVectorKP.hpp>

namespace HTTPP
{
namespace HTTP
{
namespace client
{

struct Response
{
    HTTPP::HTTP::HttpCode code;
    std::vector<Header> headers;
    auto getSortedHeaders() const -> decltype(UTILS::create_sorted_vector(headers))
    {
        return UTILS::create_sorted_vector(headers);
    }

    std::vector<char> body;
    Request request;
};

} // namespace client
} // namespace HTTP
} // namespace HTTPP

#endif // _HTTPP__HTTP_CLIENT_RESPONSE_HPP_

