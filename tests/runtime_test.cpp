/*
 * Copyright © 2015Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <com/ubuntu/location/service/runtime.h>

#include <gtest/gtest.h>

#include <condition_variable>
#include <thread>

namespace culs = com::ubuntu::location::service;

TEST(Runtime, cleanly_shuts_down_threads)
{
    culs::Runtime::create();
}

TEST(Runtime, executes_service)
{
    std::mutex m;
    std::unique_lock<std::mutex> ul{m};
    std::condition_variable wc;

    bool signaled = false;

    auto rt = culs::Runtime::create();
    rt->start();
    boost::asio::deadline_timer timer{rt->service(), boost::posix_time::milliseconds(500)};
    timer.async_wait([&wc, &signaled](const boost::system::error_code&)
    {
        signaled = true;
        wc.notify_all();
    });

    auto result = wc.wait_for(ul, std::chrono::seconds{1}, [&signaled]() { return signaled; });
    EXPECT_TRUE(result);
}

TEST(Runtime, catches_exceptions_thrown_from_handlers)
{
    std::mutex m;
    std::unique_lock<std::mutex> ul{m};
    std::condition_variable wc;

    bool signaled = false;

    auto rt = culs::Runtime::create();
    rt->start();
    boost::asio::deadline_timer fast{rt->service(), boost::posix_time::milliseconds(100)};
    fast.async_wait([](const boost::system::error_code&)
    {
        throw std::runtime_error{"Should not propagate"};
    });

    boost::asio::deadline_timer slow{rt->service(), boost::posix_time::milliseconds(500)};
    slow.async_wait([&wc, &signaled](const boost::system::error_code&)
    {
        signaled = true;
        wc.notify_all();
    });

    auto result = wc.wait_for(ul, std::chrono::seconds{1}, [&signaled]() { return signaled; });
    EXPECT_TRUE(result);
}

// sets_up_pool_of_threads ensures that the pool size 
// passed to the Runtime on construction is honored. The idea is simple:
// We set up two deadline timers, fast and slow. fast fires before slow,
// with fast blocking in a wait on a common condition_variable. When slow 
// fires, it notifies the condition variable, thereby unblocking the handler of fast,
// enabling clean shutdown without errors and timeouts. This only works if the 
// pool contains at least 2 threads. Otherwise, the handler of slow would not be executed
// until the handler of fast times out in the wait, marking the test as failed.
TEST(Runtime, sets_up_pool_of_threads)
{
    struct State
    {
        bool signaled{false};
        std::mutex m;
        std::condition_variable wc;
    };

    auto state = std::make_shared<State>();

    auto rt = culs::Runtime::create(2);
    rt->start();
    boost::asio::deadline_timer fast{rt->service(), boost::posix_time::milliseconds(100)};
    fast.async_wait([state](const boost::system::error_code&)
    {
        std::unique_lock<std::mutex> ul{state->m};
        EXPECT_TRUE(state->wc.wait_for(ul, std::chrono::seconds{1}, [state]() { return state->signaled; }));
    });

    boost::asio::deadline_timer slow{rt->service(), boost::posix_time::milliseconds(500)};
    slow.async_wait([state](const boost::system::error_code&)
    {
        state->signaled = true;
        state->wc.notify_all();
    });

    std::unique_lock<std::mutex> ul{state->m};
    auto result = state->wc.wait_for(ul, std::chrono::seconds{1}, [state]() { return state->signaled; });
    EXPECT_TRUE(result);
}
