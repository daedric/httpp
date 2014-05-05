/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP__DETAIL_CONFIG_HPP_
# define _HTTPP__DETAIL_CONFIG_HPP_

# include <type_traits>

# define HTTPP_USE_BOOST_PROMISE 1

# if HTTPP_USE_BOOST_PROMISE
#  include <boost/thread/future.hpp>
#  include <boost/exception/all.hpp>
#  define HTTPP_PROMISE_NAMESPACE boost
# else
# include <future>
#  define HTTPP_PROMISE_NAMESPACE std
# endif

namespace HTTPP
{

namespace detail
{

template <typename T>
using Promise = HTTPP_PROMISE_NAMESPACE::promise<T>;

template <typename T>
using Future = decltype(std::declval<Promise<T>>().get_future());

# if HTTPP_USE_BOOST_PROMISE

using ExceptionPtr = boost::exception_ptr;

template <typename T>
static inline ExceptionPtr
make_exception_ptr(const T& ex) noexcept(noexcept(boost::copy_exception(ex)))
{
    return boost::copy_exception(ex);
}

static inline ExceptionPtr
current_exception() noexcept(noexcept(boost::current_exception()))
{
    return boost::current_exception();
}

[[noreturn]] static void inline rethrow_exception(const ExceptionPtr& ex)
{
    boost::rethrow_exception(ex);
}

# else

using ExceptionPtr = std::exception_ptr;

template <typename T>
static inline ExceptionPtr make_exception_ptr(const T& ex)
{
    return std::make_exception_ptr(ex);
}

static inline ExceptionPtr
current_exception() noexcept(noexcept(std::current_exception()))
{
    return std::current_exception();
}

[[noreturn]] static void inline rethrow_exception(const ExceptionPtr& ex)
{
    std::rethrow_exception(ex);
}

# endif

} // namespace detail
} // namespace HTTPP

#endif
