#pragma once

#include <cstring>
#include <functional>
#include <algorithm>
#include <memory>

#include <boost/system/error_code.hpp>

namespace HTTPP
{
namespace HTTP
{
class Connection;

namespace helper
{

struct ReadWholeRequest
{
    using Handle = std::unique_ptr<ReadWholeRequest>;
    using Callback =
        std::function<void(Handle, const boost::system::error_code&)>;

    ReadWholeRequest(Connection* conn, Callback, size_t size_limit = 0);

    void start();
    void operator()(boost::system::error_code const& errc,
                    const char* data,
                    size_t len);

    Connection* connection;
    Callback cb;
    std::vector<char> body;
    size_t size_limit = 0;
};

} // namespace helper

template <typename Callable>
void read_whole_request(Connection* conn,
                        Callable&& callable,
                        size_t size_limit = 0)
{
    auto h = new helper::ReadWholeRequest(conn, std::forward<Callable>(callable),
                                          size_limit);
    h->start();
}

} // namespace HTTP
} // namespace HTTPP
