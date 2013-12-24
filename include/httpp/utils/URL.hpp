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

static inline void url_decode(std::string& fragment)
{
    try
    {
        std::replace(std::begin(fragment), std::end(fragment), '+', ' ');

        size_t idx = fragment.find('%');
        while (idx != std::string::npos)
        {
            const std::string& data = fragment.substr(idx + 1, 2);
            int encoded_char = std::stoi(data, nullptr, 16);
            fragment.replace(idx, 3, 1, static_cast<char>(encoded_char));
            idx = fragment.find('%', idx + 1);
        }
    }
    catch (std::exception const& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Cannot url_decode: " << fragment
                                 << ", exception: " << ex.what();
    }
}

static inline std::string url_decode(const std::string& fragment)
{
    std::string copy = fragment;
    url_decode(copy);
    return copy;
}

} // namespace UTILS
} // namespace HTTPP

#endif // ! HTTPP_UTILS_URL_HPP_
