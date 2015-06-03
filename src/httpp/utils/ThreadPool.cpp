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
, name_(std::move(pool.name_))
{
    running_threads_.store(pool.running_threads_.load());
    pool.running_ = false;
}

void ThreadPool::start(ThreadInit fct)
{
    if (running_)
    {
        return;
    }

    work_.reset(new boost::asio::io_service::work(*service_));
    for (size_t i = 0; i < nb_thread_; ++i)
    {
        threads_.emplace_back(std::bind(&ThreadPool::run, this,
                    [this, fct, i]
                    {
                        if (name_.empty())
                        {
                            setCurrentThreadName("Pool thread #" +
                                                 std::to_string(i));
                        }
                        else
                        {
                            setCurrentThreadName(name_ + " #" + std::to_string(i));
                        }

                        if (fct)
                        {
                            BOOST_LOG_TRIVIAL(debug) << "call init fct " << name_;
                            fct();
                        }
                    }
                    ));
    }

    while (running_threads_ != nb_thread_)
    {
        std::this_thread::yield();
    }

    running_ = true;
}

void ThreadPool::run(ThreadInit fct)
{
    BOOST_LOG_TRIVIAL(debug) << "start thread";

    if (fct)
    {
        fct();
    }

    BOOST_LOG_TRIVIAL(debug) << "call run()";
    ++running_threads_;

    this->service_->reset();
    this->service_->run();
    BOOST_LOG_TRIVIAL(debug) << "is stopping";
    --running_threads_;
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

bool ThreadPool::runningInPool() const noexcept
{
    auto current_id = std::this_thread::get_id();
    for (const auto& thread : threads_)
    {
        if (thread.get_id() == current_id)
        {
            return true;
        }
    }

    return false;
}

} // namespace UTILS
} // namespace HTTPP
