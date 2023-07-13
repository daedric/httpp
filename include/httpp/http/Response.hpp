/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_HTPP_RESPONSE_HPP_
#define _HTTPP_HTPP_RESPONSE_HPP_

#include <cstdio>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <boost/asio.hpp>

#include "Protocol.hpp"

namespace HTTPP
{
namespace HTTP
{

class Response
{
    static const char HTTP_START[9];
    static const char SPACE[1];
    static const char HTTP_DELIMITER[2];
    static const char HEADER_SEPARATOR[2];
    static const char END_OF_STREAM_MARKER[5];

public:
    // A ChunkedResponseCallback is responsible for generating individual
    // "chunks" of a response. Each call to the ChunkedResponseCallback should
    // return a string representing the content for an individual chunk. An
    // empty string signifying the end of the response.
    using ChunkedResponseCallback = std::function<std::string_view()>;

    Response() = default;
    Response(HttpCode code);
    Response(HttpCode code, std::string_view body);
    Response(HttpCode code, ChunkedResponseCallback&& callback);

    Response& setCode(HttpCode code)
    {
        code_ = code;
        return *this;
    }

    HttpCode getCode() const noexcept
    {
        return code_;
    }

    void clear();

    Response& addHeader(std::string k, std::string v);
    Response& setBody(std::string_view body);
    Response& setBody(ChunkedResponseCallback callback);

    template <typename Writer, typename WriteHandler>
    void sendResponse(Writer& writer, WriteHandler&& writeHandler)
    {
        using namespace std::string_view_literals;
        buffers_.clear();
        buffers_.reserve(5 + 1 + 4 * (headers_.size() + 1) + 1);

        { // HTTP/1.1
            buffers_.emplace_back(boost::asio::buffer(HTTP_START));
            auto n = std::snprintf(code_str_, 4, "%i", int(code_));
            buffers_.emplace_back(boost::asio::buffer(code_str_, n));
            buffers_.emplace_back(boost::asio::buffer(SPACE));

            auto message = getDefaultMessage(code_);
            buffers_.emplace_back(boost::asio::buffer(message));
        }

        buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));
        for (const auto& header : headers_)
        {
            buffers_.emplace_back(boost::asio::buffer(header.first));
            buffers_.emplace_back(boost::asio::buffer(HEADER_SEPARATOR));
            buffers_.emplace_back(boost::asio::buffer(header.second));
            buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));
        }

        {
            if (is_chunked_enconding())
            {
                static const std::string_view te = "Transfer-Encoding"sv;
                static const std::string_view chunked = "chunked"sv;
                buffers_.emplace_back(boost::asio::buffer(te));
                buffers_.emplace_back(boost::asio::buffer(HEADER_SEPARATOR));
                buffers_.emplace_back(boost::asio::buffer(chunked));
                buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));
            }
            else
            {
                static const std::string_view cl = "Content-Length"sv;
                buffers_.emplace_back(boost::asio::buffer(cl));
                buffers_.emplace_back(boost::asio::buffer(HEADER_SEPARATOR));

                auto n =
                    std::snprintf(body_size_, sizeof(body_size_), "%zu", body_.size());
                buffers_.emplace_back(boost::asio::buffer(body_size_, n));
                buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));
            }
        }

        buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));

        if (!is_chunked_enconding())
        {
            // response is non-chunked, send everything at once.
            if (!body_.empty())
            {
                buffers_.emplace_back(boost::asio::buffer(body_));
            }
            boost::asio::async_write(writer, buffers_, writeHandler);
        }
        else
        {
            // send headers, then each chunks individually.
            boost::asio::async_write(
                writer,
                buffers_,
                [this, &writer, writeHandler](const boost::system::error_code& ec, size_t size)
                {
                    // if there was an error sending the headers, notify the
                    // caller, otherwise start sending the chunks.
                    if (ec)
                    {
                        writeHandler(ec, size);
                    }
                    else
                    {
                        write_chunk(writer, writeHandler);
                    }
                }
            );
        }
    }

    bool connectionShouldBeClosed() const
    {
        return should_be_closed_;
    }

    Response& connectionShouldBeClosed(bool b)
    {
        should_be_closed_ = b;
        return *this;
    }

    std::vector<char>& mutable_body() noexcept
    {
        return body_;
    }

    const std::vector<char>& body() const noexcept
    {
        return body_;
    }

    bool isComplete() const
    {
        return code_ != HttpCode::Continue;
    }

private:
    // Sends individual chunks for a chunked response, until the end-of-stream
    // is sent or an error occurs.
    template <typename Writer, typename WriteHandler>
    void write_chunk(Writer& writer, WriteHandler writeHandler)
    {
        if (!is_chunked_enconding())
        {
            throw std::runtime_error(
                "Call to write_chunk for a response which "
                "is not using chunked encoding"
            );
        }

        // try to generate the next chunk. This may block.
        current_chunk_ = chunkedBodyCallback_();

        if (!current_chunk_.empty())
        {
            // Format the chunk header and chunk body.
            buffers_.clear();
            buffers_.reserve(4);

            std::snprintf(
                current_chunk_header_,
                sizeof(current_chunk_header_),
                "%zx",
                current_chunk_.size()
            );

            buffers_.emplace_back(boost::asio::buffer(current_chunk_header_));
            buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));
            buffers_.emplace_back(boost::asio::buffer(current_chunk_));
            buffers_.emplace_back(boost::asio::buffer(HTTP_DELIMITER));

            boost::asio::async_write(
                writer,
                buffers_,
                [this, &writer, writeHandler](const boost::system::error_code& ec, size_t size)
                {
                    if (ec)
                    {
                        // notify the original caller that an error occured
                        // during sending.
                        writeHandler(ec, size);
                    }
                    else
                    {
                        // write the next chunk.
                        write_chunk(writer, writeHandler);
                    }
                }
            );
        }
        else
        {
            // Send end of stream marker, notify the client handler that the
            // response is complete, potentially with an error if the
            // this write fails.
            boost::asio::async_write(writer, boost::asio::buffer(END_OF_STREAM_MARKER), writeHandler);
        }
    }

    bool is_chunked_enconding() const
    {
        return chunkedBodyCallback_ != nullptr;
    }

private:
    std::vector<boost::asio::const_buffer> buffers_;
    HttpCode code_ = HttpCode::Ok;
    char code_str_[4];

    std::vector<char> body_;
    char body_size_[16];
    ChunkedResponseCallback chunkedBodyCallback_;
    char current_chunk_header_[16];
    std::string_view current_chunk_;
    std::vector<Header> headers_;
    bool should_be_closed_ = false;
    std::string status_string_;
};

} // namespace HTTP
} // namespace HTTPP

#endif // ! _HTTPP_HTPP_RESPONSE_HPP_
