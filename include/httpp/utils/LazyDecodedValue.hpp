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
# include <iosfwd>
# include <boost/utility/string_ref.hpp>

namespace HTTPP
{
namespace UTILS
{

// This class is not thread safe
class LazyDecodedValue
{
public:
    LazyDecodedValue() = default;
    LazyDecodedValue(const char* str, size_t len = 0);
    LazyDecodedValue(boost::string_ref str);
    ~LazyDecodedValue() = default;

    LazyDecodedValue(const LazyDecodedValue&) = default;
    LazyDecodedValue(LazyDecodedValue&&) = default;
    LazyDecodedValue& operator=(const LazyDecodedValue&) = default;
    LazyDecodedValue& operator=(LazyDecodedValue&&) = default;

    bool operator==(const LazyDecodedValue& rhs) const noexcept;

    operator const std::string&() const;
    const std::string& string() const;

    const boost::string_ref& ref() const noexcept;

private:
    boost::string_ref raw_value_;
    mutable std::string decoded_value_;
};


bool operator==(const LazyDecodedValue&, const char*);
bool operator==(const char*, const LazyDecodedValue&);

bool operator==(const LazyDecodedValue&, const std::string&);
bool operator==(const std::string&, const LazyDecodedValue&);

bool operator==(const LazyDecodedValue&, const boost::string_ref&);
bool operator==(const boost::string_ref&, const LazyDecodedValue&);

std::string to_string(const LazyDecodedValue&);
std::ostream& operator<<(std::ostream&, const LazyDecodedValue&);

} // namespace UTILS
} // namespace HTTPP

#endif // HTTPP_UTILS_LAZYDECODEDVALUE_HPP
