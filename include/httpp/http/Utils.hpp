/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_UTILS_HPP_
# define _HTTPP_HTPP_UTILS_HPP_

# include "Request.hpp"
# include "Response.hpp"

namespace HTTPP
{
namespace HTTP
{

void setShouldConnectionBeClosed(const Request& request, Response& response);

} // namespace HTTP
} // namespace HTTPP

#endif // !_HTTPP_HTPP_UTILS_HPP_
