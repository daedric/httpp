/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_THREAD_POOL_HPP_
# define _HTTPP_UTILS_THREAD_POOL_HPP_

# include <cstddef>
# include <vector>
# include <thread>
# include <memory>
# include <functional>
# include <string>
# include <atomic>

# include <boost/asio/io_service.hpp>
# include <boost/asio/deadline_timer.hpp>
# include <boost/asio/strand.hpp>

# include "Thread.hpp"

namespace HTTPP
{
namespace UTILS
{

class ThreadPool
{
public:
    using ThreadInit = std::function<void()>;

public:
    ThreadPool(size_t nb_thread, const std::string& name = "");
    ThreadPool(size_t nb_thread,
               boost::asio::io_service& service,
               const std::string& name = "");
    ~ThreadPool();

    ThreadPool(ThreadPool&&);
    ThreadPool& operator=(ThreadPool&&) = delete;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename Callable>
    void post(Callable&& callable)
    {
        service_->post(std::forward<Callable>(callable));
    }

    template <typename Callable>
    void dispatch(Callable&& callable)
    {
        service_->dispatch(std::forward<Callable>(callable));
    }

    bool runningInPool() const noexcept;

    void start(ThreadInit fct = ThreadInit());
    void stop();

    using Strand = boost::asio::io_service::strand;
    using Timer = boost::asio::deadline_timer;

    boost::asio::io_service& getService()
    {
        return *service_;
    }

private:
    void run(ThreadInit fct);

private:
    std::shared_ptr<boost::asio::io_service> service_;
    bool running_ = false;
    size_t nb_thread_;
    std::unique_ptr<boost::asio::io_service::work> work_;
    std::vector<std::thread> threads_;
    std::string name_;
    std::atomic_uint running_threads_ = { 0 };
};

}
}

#endif // ! HTTPP_UTILS_THREAD_POOL_HPP_

