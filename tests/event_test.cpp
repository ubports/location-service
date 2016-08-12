/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/event.h>

#include <gtest/gtest.h>

#include <thread>
#include <vector>

namespace
{
struct DoesNotExist { static constexpr const char* name{"DoesNotExist"};};
}

TEST(Event, registration_yields_value_geq_than_first_user_defined_type)
{
    auto type = location::Event::register_type<DoesNotExist>(DoesNotExist::name);
    EXPECT_GE(static_cast<std::size_t>(type), static_cast<std::size_t>(location::Event::first_user_defined_type));
}

TEST(Event, registration_returns_same_type_for_multiple_invocations)
{
    auto type = location::Event::register_type<DoesNotExist>(DoesNotExist::name);
    EXPECT_EQ(type, location::Event::register_type<DoesNotExist>(DoesNotExist::name));
}

TEST(Event, registration_returns_same_type_for_multiple_invocations_for_multiple_threads)
{
    auto f = []()
    {
        auto type = location::Event::register_type<DoesNotExist>(DoesNotExist::name);
        for(auto i = 0; i < 100; i++)
            EXPECT_EQ(type, location::Event::register_type<DoesNotExist>(DoesNotExist::name));
    };

    std::vector<std::thread> threads;
    for (auto i = 0; i < 10; i++)
        threads.emplace_back(std::thread(f));
    for (auto i = 0; i < 10; i++)
        if (threads[i].joinable()) threads[i].join();
}
