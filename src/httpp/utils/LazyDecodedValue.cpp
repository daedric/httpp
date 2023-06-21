/*
 * File: src/httpp/utils/LazyDecodedValue.cpp
 * Part of httpp.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */
#include "httpp/utils/LazyDecodedValue.hpp"

#include <cstring>
#include <ostream>

#include "httpp/utils/URL.hpp"

namespace HTTPP
{
namespace UTILS
{

LazyDecodedValue::LazyDecodedValue(std::string_view str)
: raw_value_(std::move(str))
{
}

LazyDecodedValue::LazyDecodedValue(const char* str, size_t len)
: raw_value_(str, len ? len : ::strlen(str))
{
}

bool LazyDecodedValue::operator==(const LazyDecodedValue& rhs) const noexcept
{
    return raw_value_ == rhs.raw_value_;
}

LazyDecodedValue::operator const std::string&() const
{
    return string();
}

const std::string& LazyDecodedValue::string() const
{
    if (decoded_value_.empty())
    {
        decoded_value_ = url_decode(raw_value_.data(), raw_value_.end());
    }

    return decoded_value_;
}

std::string_view LazyDecodedValue::raw() const noexcept
{
    return raw_value_;
}

std::string to_string(const LazyDecodedValue& val)
{
    return val.string();
}

bool operator==(const LazyDecodedValue& lhs, const char* rhs)
{
    return lhs.string() == rhs;
}

bool operator==(const char* lhs, const LazyDecodedValue& rhs)
{
    return lhs == rhs.string();
}

bool operator==(const LazyDecodedValue& lhs, std::string_view rhs)
{
    return lhs.string() == rhs;
}

bool operator==(std::string_view lhs, const LazyDecodedValue& rhs)
{
    return lhs == rhs.string();
}

std::ostream& operator<<(std::ostream& os, const LazyDecodedValue& v)
{
    return os << v.string();
}

} // namespace UTILS
} // namespace HTTPP
