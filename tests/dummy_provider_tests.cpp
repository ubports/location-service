/*
 * Copyright © 2012-2013 Canonical Ltd.
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

#include <location/providers/dummy/provider.h>

#include <gtest/gtest.h>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace location = location;

TEST(DummyProvider, ConstructionWithConfigurationWorks)
{
    const double lat = 51.;
    const double lon = 7.;

    location::Configuration config;
    config.put(location::providers::dummy::Configuration::key_update_period(),
               1000);
    config.put(location::providers::dummy::Configuration::key_reference_position_lat(),
               lat);
    config.put(location::providers::dummy::Configuration::key_reference_position_lon(),
               lon);

    auto instance = location::providers::dummy::Provider::create_instance(config);

    EXPECT_NE(std::shared_ptr<location::Provider>(), instance);
}

TEST(DummyProvider, ConstructionWithConfigurationWorksAndDeliversCorrectUpdates)
{
    std::mutex guard;
    std::condition_variable wait_condition;

    const double lat = 51.;
    const double lon = 7.;

    location::Position reference_position
    {
        location::wgs84::Latitude{lat * location::units::Degrees},
        location::wgs84::Longitude{lon * location::units::Degrees}
    };

    location::providers::dummy::Configuration config;
    config.reference_position = reference_position;
    config.update_period = std::chrono::milliseconds{500};

    location::providers::dummy::Provider provider(config);

    provider.start_position_updates();

    location::Position position;
    provider.subscribe_to_position_updates([&position, &wait_condition](const location::Update<location::Position>& update)
    {
        position = update.value;
        wait_condition.notify_all();
    });

    std::unique_lock<std::mutex> ul(guard);
    wait_condition.wait_for(ul, std::chrono::seconds{1});

    EXPECT_EQ(reference_position, position);

    provider.stop_position_updates();
}
