/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2016 Thomas Sanchez.  All rights reserved.
 *
 */

#pragma once

#include <array>
#include <functional>
#include <stdexcept>

#include <boost/container/flat_map.hpp>

#include <commonpp/core/Utils.hpp>

#include "Protocol.hpp"
#include "helper/ReadWholeRequest.hpp"

namespace HTTPP
{

class HttpServer;

namespace HTTP
{

class Connection;

struct Route
{
    enum class RouteType
    {
        WithBody,
        WithoutBody,
    };

    using AllowedMethod = std::array<bool, 8>;
    using WithoutBodyHandler = std::function<void(HTTP::Connection*)>;
    using WithBodyHandler =
        std::function<void(HTTP::helper::ReadWholeRequest::Handle)>;

public:
    Route()
    {
        allowed_method.fill(false);
    }
    
    Route& upon(HTTP::Method h);

    template <typename... Tail>
    Route& upon(HTTP::Method h, Tail... tail)
    {
        upon(h);
        upon(tail...);
        return *this;
    }

    Route& withBody();
    Route& withoutBody();
    Route& dispatch(WithBodyHandler hndl);
    Route& dispatch(WithoutBodyHandler hndl);
    bool handle(HTTP::Connection* conn) const;

public:
    RouteType type = RouteType::WithoutBody;
    AllowedMethod allowed_method;

    WithoutBodyHandler without_body_hndl;
    WithBodyHandler with_body_handler;
};

class RestDispatcher
{

public:
    RestDispatcher(HttpServer& server);
    ~RestDispatcher();

    template <HTTP::Method... method>
    void add(std::string path, Route::WithoutBodyHandler hndl)
    {
        static_assert(sizeof...(method) > 0, "At least one method is required");
        Route route;
        route.withoutBody().upon(method...).dispatch(std::move(hndl));

        table_.emplace(std::move(path), std::move(route));
    }

    template <HTTP::Method... method>
    void add(std::string path, Route::WithBodyHandler hndl)
    {
        static_assert(sizeof...(method) > 0, "At least one method is required");
        Route route;
        route.withBody().upon(method...).dispatch(std::move(hndl));

        table_.emplace(std::move(path), std::move(route));
    }

    size_t size() const
    {
        return table_.size();
    }

private:
    void sink(HTTP::Connection* conn);

private:
    HttpServer& server_;
    boost::container::flat_multimap<std::string, Route> table_;
};

} // namespace HTTP
} // namespace HTTPP
