#include "httpp/http/helper/ReadWholeRequest.hpp"

#include <algorithm>
#include <cstring>

#include "httpp/http/Connection.hpp"

#ifdef WIN32
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

namespace HTTPP
{
namespace HTTP
{
namespace helper
{

ReadWholeRequest::ReadWholeRequest(Connection* conn,
                                   std::vector<char>& dest,
                                   Callback cb,
                                   size_t size_limit)
: connection(conn)
, cb(std::move(cb))
, body(dest)
, size_limit(size_limit)
{
}

ReadWholeRequest::ReadWholeRequest(Connection* conn,
                                   Callback cb,
                                   size_t size_limit)
: connection(conn)
, cb(std::move(cb))
, fallback(new std::vector<char>())
, body(*fallback)
, size_limit(size_limit)
{
}

void ReadWholeRequest::start()
{
    auto& request = connection->request();
    auto it = std::find_if(
        request.headers.begin(), request.headers.end(), [](const HeaderRef& s) {
            return ::strncasecmp("Content-Length", s.first.data(), 14) == 0;
        });

    size_t size = 0;
    if (it != request.headers.end())
    {
        size = atoi(it->second.data());
    }

    if (size_limit && size > size_limit)
    {
        connection->response()
            .setCode(HttpCode::RequestEntityTooLarge)
            .setBody("Size is limited to: " + std::to_string(size_limit));

        // Do not send a response, it is up to the handle to decide what to do.
        cb(Handle(this), boost::asio::error::message_size);
        return;
    }

    if (size)
    {
        body.resize(size);
        connection->read(size, body.data(), std::ref(*this));
    }
    else
    {
        cb(Handle(this), boost::system::error_code());
    }
}

void ReadWholeRequest::operator()(boost::system::error_code const& errc)
{
    cb(Handle(this), errc);
}

} // namespace helper
} // namespace HTTP
} // namespace HTTPP
