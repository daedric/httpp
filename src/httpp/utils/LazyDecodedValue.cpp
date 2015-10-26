#include "httpp/utils/LazyDecodedValue.hpp"

#include "httpp/utils/URL.hpp"

namespace HTTPP
{
namespace UTILS
{

LazyDecodedValue::LazyDecodedValue(boost::string_ref str)
: raw_value_(std::move(str))
{
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

std::string to_string(const LazyDecodedValue& val)
{
    return val.string();
}

} // namespace UTILS
} // namespace HTTPP
