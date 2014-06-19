/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/utils/Thread.hpp"

#if HAVE_SYS_PRCTL_H
# include <sys/prctl.h>
# include <cerrno>
# include <cstring>
#endif

#include <thread>
#include <sstream>

#include <boost/log/trivial.hpp>

#if !HAS_THREAD_LOCAL_SPECIFIER
# include <boost/thread/tss.hpp>
#endif


namespace HTTPP
{
namespace UTILS
{

#if HAS_THREAD_LOCAL_SPECIFIER
static thread_local std::string current_name;
#else
static boost::thread_specific_ptr<std::string> current_name;
#endif

void setCurrentThreadName(const std::string& name)
{
#if HAS_THREAD_LOCAL_SPECIFIER
    current_name = name;
#else
    current_name.reset(new std::string(name));
#endif

#if HAVE_SYS_PRCTL_H
    auto short_thread_name = name.substr(0, 15);
    if (prctl(PR_SET_NAME, short_thread_name.c_str()))
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Cannot set the custom thread name: " << short_thread_name
            << ", errno: " << errno << ", msg: " << strerror(errno);
    }
#endif
}

const std::string& getCurrentThreadName()
{
#if HAS_THREAD_LOCAL_SPECIFIER
    if (!current_name.empty())
    {
        return current_name;
    }
#else
    static const std::string empty;
    if (current_name.get())
    {
        return *current_name;
    }
#endif

    std::ostringstream out;
    out << std::this_thread::get_id();
    setCurrentThreadName(out.str());

    return getCurrentThreadName();
}

} // namespace utils
} // namespace httpp
