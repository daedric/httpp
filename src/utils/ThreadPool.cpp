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

namespace HTTPP
{
namespace UTILS
{

ThreadPool::ThreadPool(size_t nb_thread, boost::asio::io_service& service)
: service_(service)
, nb_thread_(nb_thread)
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

    work_.reset(new boost::asio::io_service::work(service_));
    running_ = true;

    std::atomic<std::size_t> init_called = {0};
    for (size_t i = 0; i < nb_thread_; ++i)
    {
        threads_.emplace_back([this, fct, &init_called]
                              {
                                  if (fct)
                                  {
                                      fct();
                                  }
                                  ++init_called;
                                  this->service_.run();
                              });
    }

    while (init_called != nb_thread_)
    {
        std::this_thread::yield();
    }
}

void ThreadPool::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;
    work_.reset();
    service_.stop();
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
