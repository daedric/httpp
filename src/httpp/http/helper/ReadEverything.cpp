#include "httpp/http/helper/ReadEverything.hpp"

#include "httpp/http/Connection.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace helper
{

ReadEverything::ReadEverything(
    Connection* conn,
    std::function<void(ReadEverything*, const boost::system::error_code&)> cb)
: connection(conn), cb(std::move(cb))
{
    auto& request = connection->request();
    auto it = std::find_if(
        request.headers.begin(), request.headers.end(), [](const HeaderRef& s) {
            return ::strncasecmp("Content-Length", s.first.data(), 14) == 0;
        });

    auto size = 0;
    if (it != request.headers.end())
    {
        size = atoi(it->second.data());
    }

    if (size)
    {
        connection->readBody(size, std::ref(*this));
    }
    else
    {
        cb(this, boost::asio::error::eof);
    }
}

void ReadEverything::operator()(boost::system::error_code const& errc,
                                const char* data,
                                size_t len)
{
    if (errc)
    {
        cb(this, errc);
    }
    else
    {
        body.reserve(body.size() + len);
        body.insert(body.end(), data, data + len);
    }
}

} // namespace helper
} // namespace HTTP
} // namespace HTTPP
