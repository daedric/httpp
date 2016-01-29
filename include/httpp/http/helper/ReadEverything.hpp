#pragma once

#include <cstring>
#include <functional>
#include <algorithm>

#include <boost/system/error_code.hpp>

namespace HTTPP
{
namespace HTTP
{
class Connection;

namespace helper
{

struct ReadEverything
{
    ReadEverything(
        Connection* conn,
        std::function<void(ReadEverything*, const boost::system::error_code&)> cb);

    void operator()(boost::system::error_code const& errc,
                    const char* data,
                    size_t len);

    Connection* connection;
    std::function<void(ReadEverything*, const boost::system::error_code&)> cb;
    std::vector<char> body;
};

} // namespace helper

template <typename Callable>
void read_everything(Connection* conn, Callable&& callable)
{
    new helper::ReadEverything(conn, std::forward<Callable>(callable));
}


} // namespace HTTP
} // namespace HTTPP
