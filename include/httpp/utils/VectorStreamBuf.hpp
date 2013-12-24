/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_VECTOR_STREAMBUF_HPP
# define _HTTPP_UTILS_VECTOR_STREAMBUF_HPP

# include <cstring>
# include <streambuf>
# include <vector>

namespace HTTPP
{
namespace UTILS
{

class VectorStreamBuf : public std::streambuf
{
public:

    VectorStreamBuf(std::vector<char>& vector)
    : VectorStreamBuf(vector, vector.size())
    {}

    VectorStreamBuf(std::vector<char>& vector,
                    size_t assumed_size)
    : content_(vector)
    , assumed_size_(assumed_size)
    {
        setg(content_.data(), content_.data(), content_.data() + assumed_size_);
    }

    void shrinkVector()
    {
        auto current_ptr = gptr();
        auto current_end = egptr();
        auto new_size = current_end - current_ptr;

        memmove(content_.data(), current_ptr, new_size);
        content_.resize(new_size);
    }

private:

    int_type underflow()
    {
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        return traits_type::eof();
    }

    std::vector<char>& content_;
    const size_t assumed_size_;
};

} // namespace UTILS
} // namespace HTTPP

#endif // !_HTTPP_UTILS_VECTOR_STREAMBUF_HPP
