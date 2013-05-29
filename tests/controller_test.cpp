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
#include "com/ubuntu/location/provider.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;

namespace
{
struct MockProvider : public cul::Provider
{
	MockProvider() = default;

	MOCK_METHOD0(start_position_updates, void());
    MOCK_METHOD0(stop_position_updates, void());

    MOCK_METHOD0(start_heading_updates, void());
    MOCK_METHOD0(stop_heading_updates, void());

    MOCK_METHOD0(start_velocity_updates, void());
    MOCK_METHOD0(stop_velocity_updates, void());

    void inject_update(const cul::Update<cul::Position>& update)
    {
        deliver_position_updates(update);
    }

    void inject_update(const cul::Update<cul::Velocity>& update)
    {
        deliver_velocity_updates(update);
    }

    void inject_update(const cul::Update<cul::Heading>& update)
    {
        deliver_heading_updates(update);
    }
};

auto timestamp = com::ubuntu::location::Clock::now();

com::ubuntu::location::Update<com::ubuntu::location::Position> reference_position_update
{
    {
        com::ubuntu::location::wgs84::Latitude{9. * com::ubuntu::location::units::Degrees},
        com::ubuntu::location::wgs84::Longitude{53. * com::ubuntu::location::units::Degrees},
        com::ubuntu::location::wgs84::Altitude{-2. * com::ubuntu::location::units::Meters}
    },
    timestamp
};

com::ubuntu::location::Update<com::ubuntu::location::Velocity> reference_velocity_update
{
    {5. * com::ubuntu::location::units::MetersPerSecond},
    timestamp
};

com::ubuntu::location::Update<com::ubuntu::location::Heading> reference_heading_update
{
    {120. * com::ubuntu::location::units::Degrees},
    timestamp
};
}

TEST(CachedUpdates, a_default_constructed_cached_value_is_invalid)
{
	cul::Provider::Controller::Cache<int> cached_value;
	EXPECT_FALSE(cached_value.is_valid());
}

TEST(CachedUpdates, updating_with_value_results_in_valid_cached_value)
{
	cul::Provider::Controller::Cache<int> cached_value;
	cached_value.update(42);
	EXPECT_TRUE(cached_value.is_valid());
	EXPECT_EQ(42, cached_value.value());
}

TEST(CachedUpdates, invalidating_a_cached_value_results_in_cache_being_invalid)
{
	cul::Provider::Controller::Cache<int> cached_value;
	cached_value.update(42);
	EXPECT_TRUE(cached_value.is_valid());
	cached_value.invalidate();
	EXPECT_FALSE(cached_value.is_valid());
}

TEST(Controller, controller_starts_and_stops_updates_on_provider)
{
	using namespace ::testing;

	MockProvider provider;

	EXPECT_CALL(provider, start_position_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, start_velocity_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, start_heading_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_position_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_velocity_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_heading_updates()).Times(Exactly(1));
	
	auto controller = provider.state_controller();

	controller->start_position_updates();
	controller->start_velocity_updates();
	controller->start_heading_updates();

	EXPECT_TRUE(controller->are_position_updates_running());
	EXPECT_TRUE(controller->are_velocity_updates_running());
	EXPECT_TRUE(controller->are_heading_updates_running());

	controller->stop_position_updates();
	controller->stop_velocity_updates();
	controller->stop_heading_updates();

	EXPECT_FALSE(controller->are_position_updates_running());
	EXPECT_FALSE(controller->are_velocity_updates_running());
	EXPECT_FALSE(controller->are_heading_updates_running());	
}

TEST(Controller, controller_starts_and_stops_updates_on_provider_only_once)
{
	using namespace ::testing;

	MockProvider provider;

	EXPECT_CALL(provider, start_position_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, start_velocity_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, start_heading_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_position_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_velocity_updates()).Times(Exactly(1));
	EXPECT_CALL(provider, stop_heading_updates()).Times(Exactly(1));
	
	auto controller = provider.state_controller();

	controller->start_position_updates();
	controller->start_velocity_updates();
	controller->start_heading_updates();

	EXPECT_TRUE(controller->are_position_updates_running());
	EXPECT_TRUE(controller->are_velocity_updates_running());
	EXPECT_TRUE(controller->are_heading_updates_running());

	controller->start_position_updates();
	controller->start_velocity_updates();
	controller->start_heading_updates();

	EXPECT_TRUE(controller->are_position_updates_running());
	EXPECT_TRUE(controller->are_velocity_updates_running());
	EXPECT_TRUE(controller->are_heading_updates_running());

	controller->stop_position_updates();
	controller->stop_velocity_updates();
	controller->stop_heading_updates();

	EXPECT_TRUE(controller->are_position_updates_running());
	EXPECT_TRUE(controller->are_velocity_updates_running());
	EXPECT_TRUE(controller->are_heading_updates_running());

	controller->stop_position_updates();
	controller->stop_velocity_updates();
	controller->stop_heading_updates();

	EXPECT_FALSE(controller->are_position_updates_running());
	EXPECT_FALSE(controller->are_velocity_updates_running());
	EXPECT_FALSE(controller->are_heading_updates_running());	
}

TEST(Controller, controller_remembers_cached_updates)
{
	using namespace ::testing;

	MockProvider provider;
	auto controller = provider.state_controller();

	EXPECT_FALSE(controller->cached_position_update().is_valid());
	EXPECT_FALSE(controller->cached_velocity_update().is_valid());
	EXPECT_FALSE(controller->cached_heading_update().is_valid());

	provider.inject_update(reference_position_update);

	EXPECT_TRUE(controller->cached_position_update().is_valid());
	EXPECT_EQ(reference_position_update, controller->cached_position_update().value());
	EXPECT_FALSE(controller->cached_velocity_update().is_valid());
	EXPECT_FALSE(controller->cached_heading_update().is_valid());

	provider.inject_update(reference_velocity_update);

	EXPECT_TRUE(controller->cached_position_update().is_valid());
	EXPECT_EQ(reference_position_update, controller->cached_position_update().value());
	EXPECT_TRUE(controller->cached_velocity_update().is_valid());
	EXPECT_EQ(reference_velocity_update, controller->cached_velocity_update().value());
	EXPECT_FALSE(controller->cached_heading_update().is_valid());

	provider.inject_update(reference_heading_update);

	EXPECT_TRUE(controller->cached_position_update().is_valid());
	EXPECT_EQ(reference_position_update, controller->cached_position_update().value());
	EXPECT_TRUE(controller->cached_velocity_update().is_valid());
	EXPECT_EQ(reference_velocity_update, controller->cached_velocity_update().value());
	EXPECT_TRUE(controller->cached_heading_update().is_valid());
	EXPECT_EQ(reference_heading_update, controller->cached_heading_update().value());
}

