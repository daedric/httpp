/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_THREAD_HPP_
# define _HTTPP_UTILS_THREAD_HPP_

# include <string>

namespace HTTPP
{
namespace UTILS
{

void setCurrentThreadName(const std::string& name);
const std::string& getCurrentThreadName();

} // namespace utils
} // namespace httpp

#endif // ! HTTPP_UTILS_THREAD_HPP_
