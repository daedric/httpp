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
#define HTTPP_UTILS_LAZYDECODEDVALUE_HPP

#include <iosfwd>
#include <string>

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
    LazyDecodedValue(std::string_view str);
    ~LazyDecodedValue() = default;

    LazyDecodedValue(const LazyDecodedValue&) = default;
    LazyDecodedValue(LazyDecodedValue&&) = default;
    LazyDecodedValue& operator=(const LazyDecodedValue&) = default;
    LazyDecodedValue& operator=(LazyDecodedValue&&) = default;

    bool operator==(const LazyDecodedValue& rhs) const noexcept;

    operator const std::string&() const;
    const std::string& string() const;

    std::string_view raw() const noexcept;

private:
    std::string_view raw_value_;
    mutable std::string decoded_value_;
};

bool operator==(const LazyDecodedValue&, const char*);
bool operator==(const char*, const LazyDecodedValue&);

bool operator==(const LazyDecodedValue&, std::string_view);
bool operator==(std::string_view, const LazyDecodedValue&);

std::string to_string(const LazyDecodedValue&);
std::ostream& operator<<(std::ostream&, const LazyDecodedValue&);

} // namespace UTILS
} // namespace HTTPP

#endif // HTTPP_UTILS_LAZYDECODEDVALUE_HPP
