/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_RESPONSE_HPP_
# define _HTTPP_HTPP_RESPONSE_HPP_

# include <functional>
# include <vector>
# include <string>

# include <boost/asio.hpp>

# include "HttpCode.hpp"

namespace HTTPP
{
namespace HTTP
{

class Response
{
    static char const HTTP_DELIMITER[2];
    static char const HEADER_SEPARATOR[2];

public:

    using Generator = std::function<std::string ()>;
    using Header = std::pair<std::string, std::string>;


    Response();
    Response(HttpCode code);
    Response(HttpCode code, const std::string& body);
    Response(HttpCode code, std::string&& body);

    template <typename Callable>
    Response(HttpCode code, Callable&& generator)
    : code_(code)
    , generator_(std::forward<Callable>(generator))
    {}

    Response& setCode(HttpCode code)
    {
        code_ = code;
        return *this;
    }

    Response& addHeader(const std::string& k, const std::string& v);
    Response& setBody(const std::string& body);

    template <typename Callable>
    Response& setGenerator(Callable&& generator)
    {
        generator_ = std::forward<Callable>(generator);
        return *this;
    }

    template <typename Writer, typename Callable>
    void sendResponse(Writer& writer, Callable&& callable)
    {
        generate_status_string();

        if (generator_)
        { // chunked response

        }
        else
        {
            finalize_response_headers();
            std::vector<boost::asio::const_buffer> buffers;
            buffers.push_back(boost::asio::buffer(status_string_));
            buffers.push_back(boost::asio::buffer(HTTP_DELIMITER));
            for (auto const& header : headers_)
            {
                buffers.push_back(boost::asio::buffer(header.first));
                buffers.push_back(boost::asio::buffer(HEADER_SEPARATOR));
                buffers.push_back(boost::asio::buffer(header.second));
                buffers.push_back(boost::asio::buffer(HTTP_DELIMITER));
            }

            buffers.push_back(boost::asio::buffer(HTTP_DELIMITER));
            buffers.push_back(boost::asio::buffer(body_));
            boost::asio::async_write(writer, buffers, callable);
        }
    }

    bool connectionShouldBeClosed() const
    {
        return should_be_closed_;
    }

    void connectionShouldBeClosed(bool b)
    {
        should_be_closed_ = b;
    }

private:
    void generate_status_string()
    {
        status_string_ = "HTTP/" + std::to_string(major_) + "." +
                         std::to_string(minor_) + " " +
                         std::to_string(static_cast<unsigned int>(code_)) +
                         " " + getDefaultMessage(code_);
    }

    void finalize_response_headers()
    {
        headers_.emplace_back("Content-Length", std::to_string(body_.size()));
    }

private:
    HttpCode code_;
    std::string body_;
    Generator generator_;
    int major_ = 1;
    int minor_ = 1;
    std::vector<Header> headers_;
    bool should_be_closed_ = false;

    std::string status_string_;

};

} // namespace HTTP
} // namespace HTTPP

#endif // ! _HTTPP_HTPP_RESPONSE_HPP_
