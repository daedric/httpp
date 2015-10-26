/*
 * File: include/httpp/utils/LazyDecodedValue.hpp
 * Part of httpp.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef HTTPP_UTILS_LAZYDECODEDVALUE_HPP
# define HTTPP_UTILS_LAZYDECODEDVALUE_HPP

# include <string>
# include <boost/utility/string_ref.hpp>

namespace HTTPP
{
namespace UTILS
{

// This class is not thread safe
class LazyDecodedValue
{
public:
    LazyDecodedValue(boost::string_ref str);
    ~LazyDecodedValue() = default;

    LazyDecodedValue(const LazyDecodedValue&) = default;
    LazyDecodedValue(LazyDecodedValue&&) = default;
    LazyDecodedValue& operator=(const LazyDecodedValue&) = default;
    LazyDecodedValue& operator=(LazyDecodedValue&&) = default;

    operator const std::string&() const;
    const std::string& string() const;

private:
    boost::string_ref raw_value_;
    mutable std::string decoded_value_;
};

std::string to_string(const LazyDecodedValue&);

} // namespace UTILS
} // namespace HTTPP

#endif // HTTPP_UTILS_LAZYDECODEDVALUE_HPP
