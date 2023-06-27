/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
 *
 */

#include "Manager.hpp"

#include <atomic>
#include <exception>
#include <functional>
#include <mutex>
#include <stdexcept>

#include "Connection.hpp"
#include "httpp/utils/Exception.hpp"

static std::once_flag curl_init_flag;

static void init_curl()
{
    GLOG(info) << "initialize libcurl";
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        throw std::runtime_error("Cannot initialize curl");
    }

    ::atexit(&::curl_global_cleanup);

    GLOG(info) << "curl initialized";
}

namespace HTTPP
{
namespace HTTP
{
namespace client
{
namespace detail
{

DECLARE_LOGGER(manager_logger, "httpp::client::Manager");

Manager::Manager(ThreadPool& io, ThreadPool& dispatch)
: handler(nullptr)
, io(io)
, dispatch(dispatch)
, timer(io.getService())
{
    std::call_once(curl_init_flag, &init_curl);
    handler = curl_multi_init();
    if (!handler)
    {
        throw std::runtime_error("Cannot initialize curl multi handle");
    }

    manager_setopt(CURLMOPT_SOCKETFUNCTION, &sock_cb);
    manager_setopt(CURLMOPT_SOCKETDATA, this);
    manager_setopt(CURLMOPT_TIMERFUNCTION, &curl_timer_cb);
    manager_setopt(CURLMOPT_TIMERDATA, this);
}

Manager::~Manager()
{
    std::vector<Future<void>> futures;
    {
        Promise<void> promise;
        auto future = promise.get_future();

        io.dispatch(
            [this, &promise, &futures]
            {
                running = false;

                auto conns = current_connections;

                for (auto& conn : conns)
                {
                    bool expected = false;
                    if (conn.first->cancelled.compare_exchange_strong(expected, true))
                    {
                        LOG(manager_logger, info)
                            << "Cancel one connection still alive";
                        futures.emplace_back(cancel_connection(conn.first));
                    }
                }

                promise.set_value();
            }
        );

        future.get();
    }

    while (!futures.empty())
    {
        auto conn_future = std::move(futures.back());
        conn_future.get();
        futures.pop_back();
    }

    {
        Promise<void> promise;
        auto future = promise.get_future();
        io.dispatch(
            [this, &promise]
            {
                checkHandles();
                promise.set_value();
            }
        );
        future.get();
    }

    boost::system::error_code ec;
    timer.cancel(ec);

    if (handler)
    {
        curl_multi_cleanup(handler);
        handler = nullptr;
    }

    if (!sockets.empty() || !current_connections.empty())
    {
        LOG(manager_logger, error)
            << "There are still curl socket opened: " << sockets.size() << ", "
            << current_connections.size();
    }
}

void Manager::timer_cb(const boost::system::error_code& error)
{
    if (!error)
    {
        int still_running = 0;
        auto rc = curl_multi_socket_action(handler, CURL_SOCKET_TIMEOUT, 0, &still_running);

        if (rc != CURLM_OK)
        {
            LOG(manager_logger, error) << "Error curl multi: " << curl_multi_strerror(rc);
            throw std::runtime_error("timer_cb error");
        }

        checkHandles();
    }
}

int Manager::curl_timer_cb(CURLM*, long timeout_ms, void* userdata)
{
    Manager* manager = (Manager*)userdata;

    if (timeout_ms > 0)
    {
        if (manager->running)
        {
            manager->timer.expires_from_now(boost::posix_time::millisec(timeout_ms));
            manager->timer.async_wait(
                std::bind(&Manager::timer_cb, manager, std::placeholders::_1)
            );
        }
        else
        {
            return -1;
        }
    }
    else
    {
        manager->io.post(
            [manager]
            {
                manager->timer.cancel();
                boost::system::error_code error;
                manager->timer_cb(error);
            }
        );
    }

    return 0;
}

int Manager::sock_cb(CURL* easy, curl_socket_t s, int what, void* multi_private, void*)
{
    Manager* manager = (Manager*)multi_private;
    LOG(manager_logger, trace) << "Manager(" << manager << ") sock_cb: "
                               << "socket: " << s << ", what: " << what;

    Connection* conn;
    auto rc = curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
    auto connection = conn->shared_from_this();

    if (what == CURL_POLL_REMOVE)
    {
        return 0;
    }

    if (rc != CURLE_OK)
    {
        throw std::runtime_error(
            "Cannot get private info:" + std::string(curl_easy_strerror(rc))
        );
    }

    connection->setSocket(s);
    connection->poll_action = what;
    manager->poll(std::move(connection), what);
    return 0;
}

void Manager::removeHandle(CURL* easy)
{
    auto rc = curl_multi_remove_handle(handler, easy);

    if (rc != CURLM_OK)
    {
        LOG(manager_logger, error)
            << "Cannot unregister easy handle: " << curl_multi_strerror(rc);
    }
}

void Manager::removeConnection(std::shared_ptr<Connection> conn)
{
    auto it = current_connections.find(conn);
    if (it == std::end(current_connections))
    {
        LOG(manager_logger, error) << "Cannot find connection: " << conn << " to delete";
        throw std::runtime_error("Cannot find connection to delete");
        return;
    }

    removeHandle(conn->handle);
    current_connections.erase(it);
}

void Manager::checkHandles()
{
    LOG(manager_logger, trace) << "CheckHandles";
    CURLMsg* msg;
    int msgs_left = 0;
    while ((msg = curl_multi_info_read(handler, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            auto easy = msg->easy_handle;
            auto result = msg->data.result;

            Connection* conn;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
            conn->poll_action = 0;

            LOG(manager_logger, trace) << "CheckHandles: Connection done: " << conn;
            auto shared_ptr = conn->shared_from_this();
            removeConnection(shared_ptr);
            shared_ptr->buildResponse(result);
        }
    }

    while (!cancelled_connections.empty())
    {
        auto cancelled = std::move(*cancelled_connections.begin());
        LOG(manager_logger, trace)
            << "CheckHandles: Connection cancel: " << cancelled.first;

        cancelled_connections.erase(cancelled_connections.begin());

        cancelled.first->poll_action = 0;
        cancelled.first->complete(HTTPP::detail::make_exception_ptr(UTILS::OperationAborted()));
        cancelled.second.set_value();

        if (current_connections.count(cancelled.first))
        {
            removeConnection(cancelled.first);
        }
        else
        {
            LOG(manager_logger, warning)
                << "Connection already deleted: " << cancelled.first;
        }
    }
}

void Manager::performOp(
    std::shared_ptr<Connection> connection, int action, const boost::system::error_code& ec
)
{
    int still_running = 0;

    if (ec)
    {
        if (ec != boost::asio::error::operation_aborted)
        {
            LOG(manager_logger, warning) << "Error on socket: " << ec.message();
            connection->complete(HTTPP::detail::make_exception_ptr(
                std::runtime_error("Error on socket: " + ec.message())
            ));
        }

        LOG(manager_logger, trace) << "Manager: " << this << ", performOp: "
                                   << "Error: " << ec.message();

        checkHandles();
        return;
    }

    auto it = current_connections.find(connection);
    if (it == std::end(current_connections))
    {
        LOG(manager_logger, trace)
            << "Manager: " << this << ", performOp: "
            << "Error: can't find connection: " << connection;
        return;
    }

    LOG(manager_logger, trace)
        << "Manager: " << this << ", performOp: "
        << "Start operation: " << action << " on connection: " << connection;
    it->second = PerformIo;

    auto socket_native = connection->socket->native_handle();

    auto rc = curl_multi_socket_action(handler, socket_native, action, &still_running);

    LOG(manager_logger, trace)
        << "Manager: " << this << ", performOp: "
        << "Operation: " << action << " finished on socket: " << connection;
    if (rc != CURLM_OK)
    {
        LOG(manager_logger, error)
            << "Error happened in perfomOp: " << curl_multi_strerror(rc);
        auto exc =
            HTTPP::detail::make_exception_ptr(std::runtime_error(curl_multi_strerror(rc)));
        connection->complete(exc);
        HTTPP::detail::rethrow_exception(exc);
    }

    checkHandles();

    if (still_running <= 0)
    {
        LOG(manager_logger, trace)
            << "No more operations are running, cancelling the timer";
        timer.cancel();
    }
    else
    {
        if (connection->poll_action)
        {
            auto it = current_connections.find(connection);
            if (it == end(current_connections))
            {
                LOG(manager_logger, trace)
                    << "Do not continue the polling on connection: " << connection
                    << ", native_socket: " << socket_native
                    << ", socket: " << connection->socket
                    << ", connection is not managed anymore";
                return;
            }

            LOG(manager_logger, trace)
                << "Continue the polling on connection: " << connection
                << ", native_socket: " << socket_native
                << ", socket: " << connection->socket;

            current_connections[connection] = Default;
            poll(connection, connection->poll_action);
        }
    }
}

void Manager::poll(std::shared_ptr<Connection> connection, int action)
{
    auto it = current_connections.find(connection);

    if (it == std::end(current_connections))
    {
        LOG(manager_logger, error) << "Cannot poll an untracked connection: " << connection;
        return;
    }

    if (it->second == Default)
    {
        current_connections[connection] = Polling;
        connection->poll(
            action, std::bind(&Manager::performOp, this, connection, action, std::placeholders::_1)
        );
    }
}

Manager::Future<void> Manager::cancel_connection(std::shared_ptr<Connection> connection)
{
    auto promise = std::make_shared<Promise<void>>();
    auto future = promise->get_future();

    io.dispatch(std::bind(
        &Manager::cancel_connection_io_thread, this, std::move(connection), std::move(promise)
    ));
    return future;
}

void Manager::cancel_connection_io_thread(
    std::shared_ptr<Connection> connection, std::shared_ptr<Promise<void>> promise_ptr
)
{
    Promise<void> promise = std::move(*promise_ptr);
    promise_ptr.reset();

    auto it = current_connections.find(connection);

    if (it == std::end(current_connections))
    {
        LOG(manager_logger, warning) << "Cannot cancel a completed connection";
        promise.set_exception(HTTPP::detail::make_exception_ptr(
            std::runtime_error("Connection already completed")
        ));
        return;
    }

    auto current_connection_state = it->second;

    if (current_connection_state == Cancelled)
    {
        LOG(manager_logger, warning) << "Connection already cancelled: " << connection;
        promise.set_value();
        return;
    }

    auto it_cancelled = cancelled_connections.emplace(connection, std::move(promise));

    if (!it_cancelled.second)
    {
        LOG(manager_logger, error)
            << "Connection is already cancelled but its status is "
               "not set to cancel: "
            << current_connection_state;

        it_cancelled.first->second.set_value();
        it->second = Cancelled;
        return;
    }

    it->second = Cancelled;
    if (current_connection_state == Polling)
    {
        LOG(manager_logger, debug) << "Connection is polling, cancel operation";
        connection->cancelPoll();
    }
}

void Manager::cancelConnection(std::shared_ptr<Connection> connection)
{
    auto future = cancel_connection(connection);
    future.get();
}

int Manager::closeSocket(curl_socket_t curl_socket)
{
    Promise<int> promise;
    auto future = promise.get_future();

    io.dispatch(
        [this, curl_socket, &promise]
        {
            auto it = sockets.find(curl_socket);
            if (it == std::end(sockets))
            {
                LOG(manager_logger, error) << "Cannot find a socket to close";
                promise.set_value(1);
                return;
            }

            LOG(manager_logger, trace)
                << "Delete close socket: " << it->second << ", curl socket: " << curl_socket
                << ", socket native_handle: " << it->second->native_handle();

            delete it->second;
            sockets.erase(it);
            promise.set_value(0);
        }
    );

    return future.get();
}

void Manager::handleRequest(Method method, Connection::ConnectionPtr conn)
{
    io.post(
        [this, method, conn]()
        {
            if (!running)
            {
                LOG(manager_logger, error)
                    << "Refuse connection, manager is stopped";
                return;
            }

            conn->dispatch = std::addressof(dispatch);

            try
            {
                conn->configureRequest(method);
            }
            catch (const std::exception& exc)
            {
                LOG(manager_logger, error)
                    << "Error when configuring the request: " << exc.what();
                conn->complete(HTTPP::detail::current_exception());
                return;
            }

            auto pair = current_connections.emplace(conn, Default);

            if (!pair.second)
            {
                LOG(manager_logger, error) << "Connection already present: " << conn;
                conn->complete(HTTPP::detail::make_exception_ptr(std::runtime_error(
                    "Cannot schedule an operation for an already "
                    "managed connection"
                )));
                return;
            }

            auto rc = curl_multi_add_handle(handler, conn->handle);
            if (rc != CURLM_OK)
            {
                std::string message = curl_multi_strerror(rc);
                LOG(manager_logger, error) << "Error scheduling a new request: " << message;
                removeConnection(conn);
                conn->complete(HTTPP::detail::make_exception_ptr(std::runtime_error(message)));
                return;
            }
        }
    );
}

} // namespace detail
} // namespace client
} // namespace HTTP
} // namespace HTTPP
