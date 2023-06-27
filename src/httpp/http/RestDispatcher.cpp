/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2016 Thomas Sanchez.  All rights reserved.
 *
 */

#include <httpp/HttpServer.hpp>
#include <httpp/http/Connection.hpp>
#include <httpp/http/RestDispatcher.hpp>
#include <httpp/http/Utils.hpp>

namespace HTTPP
{
namespace HTTP
{

Route& Route::upon(HTTP::Method h)
{
    allowed_method[commonpp::enum_to_number(h)] = true;
    return *this;
}

Route& Route::withBody()
{
    type = RouteType::WithBody;
    return *this;
}

Route& Route::withoutBody()
{
    type = RouteType::WithoutBody;
    return *this;
}

Route& Route::dispatch(WithBodyHandler hndl)
{
    if (type != RouteType::WithBody)
    {
        throw std::logic_error("Bad route type for the given handler");
    }
    with_body_handler = hndl;
    return *this;
}

Route& Route::dispatch(WithoutBodyHandler hndl)
{
    if (type != RouteType::WithoutBody)
    {
        throw std::logic_error("Bad route type for the given handler");
    }
    without_body_hndl = hndl;
    return *this;
}

bool Route::handle(HTTP::Connection* conn) const
{
    auto& request = conn->request();
    if (!allowed_method[commonpp::enum_to_number(request.method)])
    {
        return false;
    }

    switch (type)
    {
    case RouteType::WithoutBody:
        without_body_hndl(conn);
        break;
    case RouteType::WithBody:
        read_whole_request(
            conn,
            [this](helper::ReadWholeRequest::Handle handle, const boost::system::error_code& ec)
            {
                if (ec)
                {
                    HTTPP::HTTP::Connection::releaseFromHandler(handle->connection);
                    return;
                }

                this->with_body_handler(std::move(handle));
            }
        );
        break;
    }

    return true;
}

RestDispatcher::RestDispatcher(HttpServer& server)
: server_(server)
{
    server_.setSink(std::bind(&RestDispatcher::sink, this, std::placeholders::_1));
}

RestDispatcher::~RestDispatcher() = default;

struct Comparator
{
    template <typename Pair, typename StrLike>
    bool operator()(const Pair& lhs, const StrLike& rhs) const
    {
        return lhs.first < rhs;
    }
};

void RestDispatcher::sink(HTTP::Connection* conn)
{
    HTTP::setShouldConnectionBeClosed(conn->request(), conn->response());

    if (BOOST_UNLIKELY(table_.empty()))
    {
        throw std::logic_error("Dispatch table is empty");
    }

    const auto& path = conn->request().uri;
    auto it = std::lower_bound(table_.begin(), table_.end(), path, Comparator());

    for (auto end = table_.end(); it != end && it->first == path; ++it)
    {
        if (it->second.handle(conn))
        {
            return; // Properly handled
        }
    }

    conn->response()
        .setCode(HTTP::HttpCode::NotFound)
        .setBody("Unroutable request");
    conn->sendResponse();
}

} // namespace HTTP
} // namespace HTTPP
