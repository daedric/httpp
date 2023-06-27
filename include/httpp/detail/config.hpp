/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#pragma once

#include <future>
#include <type_traits>

#include <httpp/detail/generated/config.hpp>
#include <httpp/version.hpp>

#define HTTPP_RAGEL_BACKEND 0
#define HTTPP_STREAM_BACKEND 1

#ifndef HTTPP_PARSER_BACKEND
#    warning "No HTTPP Parser selected, use ragel by default"
#    define HTTPP_PARSER_BACKEND HTTPP_RAGEL_BACKEND
#endif

namespace HTTPP
{

namespace detail
{

template <typename T>
using Promise = std::promise<T>;

template <typename T>
using Future = decltype(std::declval<Promise<T>>().get_future());

using ExceptionPtr = std::exception_ptr;

template <typename T>
static inline ExceptionPtr make_exception_ptr(const T& ex)
{
    return std::make_exception_ptr(ex);
}

static inline ExceptionPtr current_exception() noexcept(noexcept(std::current_exception()))
{
    return std::current_exception();
}

[[noreturn]]
static inline void rethrow_exception(const ExceptionPtr& ex)
{
    std::rethrow_exception(ex);
}

} // namespace detail
} // namespace HTTPP
