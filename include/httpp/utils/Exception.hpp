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
# include <system_error>

# include <boost/system/error_code.hpp>

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

static inline std::system_error
convert_boost_ec_to_std_ec(boost::system::error_code const& err)
{
    if (err.category() == boost::system::system_category())
    {
        return { { err.value(), std::system_category() }, err.message() };
    }
    else if (err.category() == boost::system::generic_category())
    {
        return { { err.value(), std::generic_category() }, err.message() };
    }

    // XXX: I guess it is ok
    return { { err.value(), std::generic_category() }, err.message() };
}

} // namespace UTILS
} // namespace HTTPP

#endif
