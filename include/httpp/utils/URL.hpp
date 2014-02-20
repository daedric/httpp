/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_URL_HPP_
# define _HTTPP_UTILS_URL_HPP_

# include <string>
# include <boost/log/trivial.hpp>

namespace HTTPP
{
namespace UTILS
{

bool url_decode(std::string& fragment, bool replace_plus = true);
std::string decode(const std::string& fragment, bool replace_plus = true);

bool url_encode(std::string& fragment);
std::string encode(const std::string& fragment);

} // namespace UTILS
} // namespace HTTPP

#endif // ! HTTPP_UTILS_URL_HPP_
