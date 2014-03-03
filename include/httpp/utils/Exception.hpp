/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_EXCEPTION_HPP_
# define _HTTPP_UTILS_EXCEPTION_HPP_

# include <stdexcept>

namespace HTTPP
{
namespace UTILS
{
struct OperationAborted : public std::runtime_error
{
    OperationAborted() : std::runtime_error("Operation Aborted")
    {
    }
};

} // namespace UTILS
} // namespace HTTPP

#endif
