/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#include "httpp/utils/ThreadPool.hpp"

#include <atomic>

#include <boost/log/trivial.hpp>

namespace HTTPP
{
namespace UTILS
{

static void empty_deleter(boost::asio::io_service*)
{}

ThreadPool::ThreadPool(size_t nb_thread, const std::string& name)
: service_(std::make_shared<boost::asio::io_service>(nb_thread))
, nb_thread_(nb_thread)
, name_(name)
{
}

ThreadPool::ThreadPool(size_t nb_thread,
                       boost::asio::io_service& service,
                       const std::string& name)
: service_(std::addressof(service), &empty_deleter)
, nb_thread_(nb_thread)
, name_(name)
{
}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        stop();
    }
}

ThreadPool::ThreadPool(ThreadPool&& pool)
: service_(pool.service_)
, running_(pool.running_)
, nb_thread_(pool.nb_thread_)
, work_(std::move(pool.work_))
, threads_(std::move(pool.threads_))
{
    pool.running_ = false;
}

void ThreadPool::start(ThreadInit fct)
{
    if (running_)
    {
        return;
    }

    if (!name_.empty())
    {
        name_ = "[Pool: " + name_ + "] ";
    }

    work_.reset(new boost::asio::io_service::work(*service_));
    for (size_t i = 0; i < nb_thread_; ++i)
    {
        threads_.emplace_back(std::bind(&ThreadPool::run, this, fct));
    }

    while (running_threads_ != nb_thread_)
    {
        std::this_thread::yield();
    }

    running_ = true;
}

void ThreadPool::run(ThreadInit fct)
{
    ++running_threads_;
    BOOST_LOG_TRIVIAL(debug) << name_ << "start thread: #"
                             << std::this_thread::get_id();
    if (fct)
    {
        BOOST_LOG_TRIVIAL(debug) << name_ << "[" << std::this_thread::get_id()
                                 << "] call init fct";
        fct();
    }

    BOOST_LOG_TRIVIAL(debug) << name_ << "[" << std::this_thread::get_id()
                             << "] call run()";
    this->service_->run();
    BOOST_LOG_TRIVIAL(debug) << name_ << "[" << std::this_thread::get_id()
                             << "] is stopping";
}

void ThreadPool::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;
    work_.reset();
    service_->stop();
    for (auto& th : threads_)
    {
        if (th.joinable())
        {
            th.join();
        }
    }

    threads_.clear();
}

} // namespace UTILS
} // namespace HTTPP
