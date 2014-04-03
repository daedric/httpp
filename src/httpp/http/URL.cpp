/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/utils/URL.hpp"

#include <curl/curl.h>

namespace HTTPP
{
namespace UTILS
{

bool url_decode(std::string& fragment, bool replace_plus)
{
    char* decoded = curl_unescape(fragment.data(), fragment.size());
    if (decoded)
    {
        fragment = decoded;
        curl_free(decoded);

        if (replace_plus)
        {
            std::replace_if(std::begin(fragment),
                            std::end(fragment),
                            [](const char val)
                            { return val == '+'; },
                            ' ');
        }

        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "cannot url_decode: " << fragment;
        return false;
    }
}

std::string decode(const std::string& fragment, bool replace_plus)
{
    std::string str = fragment;
    if (url_decode(str, replace_plus))
    {
        return str;
    }

    throw std::runtime_error("cannot decode: " + fragment);
}

bool url_encode(std::string& fragment)
{
    char* encoded = curl_escape(fragment.data(), fragment.size());
    if (encoded)
    {
        fragment = encoded;
        curl_free(encoded);
        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "cannot url_encode: " << fragment;
        return false;
    }
}

std::string encode(const std::string& fragment)
{
    std::string str = fragment;
    if (url_encode(str))
    {
        return str;
    }

    throw std::runtime_error("cannot decode: " + fragment);
}

} // namespace UTILS
} // namespace HTTPP

