/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTIlocationAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <com/ubuntu/location/service/runtime.h>

#include <com/ubuntu/location/logging.h>

namespace culs = com::ubuntu::location::service;

namespace
{
// exception_safe_run runs service, catching all exceptions and
// restarting operation until an explicit shutdown has been requested.
//
// TODO(tvoss): Catching all exceptions is risky as they might signal unrecoverable
// errors. We should enable calling code to decide whether an exception should be considered
// fatal or not.
void exception_safe_run(boost::asio::io_service& service)
{
    while (true)
    {
        try
        {
            service.run();
            // a clean return from run only happens in case of
            // stop() being called (we are keeping the service alive with
            // a service::work instance).
            break;
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << e.what();
        }
        catch (...)
        {
            LOG(WARNING) << "Unknown exception caught while executing boost::asio::io_service";
        }
    }
}
}

std::shared_ptr<culs::Runtime> culs::Runtime::create(std::uint32_t pool_size)
{
    return std::shared_ptr<culs::Runtime>(new culs::Runtime(pool_size));
}

culs::Runtime::Runtime(std::uint32_t pool_size)
    : service_{pool_size},
      strand_{service_},
      keep_alive_{service_}
{
        for (unsigned int i = 0; i < pool_size; i++)
            workers_.push_back(std::thread{exception_safe_run, std::ref(service_)});
}

culs::Runtime::~Runtime()
{
    try
    {
        stop();
    } catch(...)
    {
        // Dropping all exceptions to satisfy the nothrow guarantee.
    }
}

void culs::Runtime::stop()
{
    service_.stop();

    for (auto& worker : workers_)
        if (worker.joinable())
            worker.join();
}

std::function<void(std::function<void()>)> culs::Runtime::to_dispatcher_functional()
{
    // We have to make sure that we stay alive for as long as
    // calling code requires the dispatcher to work.
    auto sp = shared_from_this();
    return [sp](std::function<void()> task)
    {
        sp->strand_.post(task);
    };
}

boost::asio::io_service& culs::Runtime::service()
{
    return service_;
}
