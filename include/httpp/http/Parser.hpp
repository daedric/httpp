/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_PARSER_HPP_
# define _HTTPP_HTPP_PARSER_HPP_

# include <iosfwd>
# include <tuple>
# include <boost/system/error_code.hpp>

namespace HTTPP
{
namespace HTTP
{

struct Request;

class Parser
{
public:
    Parser() = delete;
    static bool isComplete(const char* buffer, size_t n);
    static bool parse(std::istream& is, Request& request);
};

} // namespace HTTP
} // namespace HTTPP

#endif // !_HTTPP_HTPP_PARSER_HPP_
