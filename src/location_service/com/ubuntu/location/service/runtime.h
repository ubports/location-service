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
#ifndef COM_UBUNTU_LOCATION_SERVICE_RUNTIME_H_
#define COM_UBUNTU_LOCATION_SERVICE_RUNTIME_H_

#include <boost/asio.hpp>

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <cstdint>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
// We bundle our "global" runtime dependencies here, specifically
// a dispatcher to decouple multiple in-process providers from one
// another , forcing execution to a well known set of threads.
class Runtime : public std::enable_shared_from_this<Runtime>
{
public:
    // Our default concurrency setup.
    static constexpr const std::uint32_t worker_threads = 4;

    // create returns a Runtime instance with pool_size worker threads
    // executing the underlying service.
    static std::shared_ptr<Runtime> create(std::uint32_t pool_size = worker_threads);

    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    // Tears down the runtime, stopping all worker threads.
    ~Runtime() noexcept(true);
    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    // start executes the underlying io_service on a thread pool with
    // the size configured at creation time.
    void start();

    // stop cleanly shuts down a Runtime instance,
    // joining all worker threads.
    void stop();

    // to_dispatcher_functional returns a function for integration 
    // with components that expect a dispatcher for operation. 
    std::function<void(std::function<void()>)> to_dispatcher_functional();

    // service returns the underlying boost::asio::io_service that is executed
    // by the Runtime.
    boost::asio::io_service& service();

private:
    // Runtime constructs a new instance, firing up pool_size 
    // worker threads.
    Runtime(std::uint32_t pool_size);

    std::uint32_t pool_size_;
    boost::asio::io_service service_;
    boost::asio::io_service::strand strand_;
    boost::asio::io_service::work keep_alive_;
    std::vector<std::thread> workers_;
};
}
}
}
}

#endif // COM_UBUNTU_LOCATION_SERVICE_RUNTIME_H_
