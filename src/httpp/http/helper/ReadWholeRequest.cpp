#include "httpp/http/helper/ReadWholeRequest.hpp"

#include "httpp/http/Connection.hpp"

namespace HTTPP
{
namespace HTTP
{
namespace helper
{

ReadWholeRequest::ReadWholeRequest(Connection* conn, Callback cb, size_t size_limit)
: connection(conn), cb(std::move(cb)), size_limit(size_limit)
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
        connection->readBody(size, std::ref(*this));
    }
    else
    {
        cb(Handle(this), boost::system::error_code());
    }
}

void ReadWholeRequest::operator()(boost::system::error_code const& errc,
                                  const char* data,
                                  size_t len)
{
    if (errc || data == nullptr)
    {
        cb(Handle(this), errc);
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
