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
#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_selection_policy.h>

#include <com/ubuntu/location/connectivity/manager.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace location = com::ubuntu::location;

namespace
{
struct MockProvider : public location::Provider
{
    MockProvider() : location::Provider()
    {
    }

    MOCK_METHOD0(stop_position_updates, void());
    MOCK_METHOD0(stop_velocity_updates, void());
    MOCK_METHOD0(stop_heading_updates, void());

    MOCK_METHOD1(on_wifi_and_cell_reporting_state_changed,
                 void(location::WifiAndCellIdReportingState));
    MOCK_METHOD1(on_reference_location_updated,
                 void(const location::Update<location::Position>&));
    MOCK_METHOD1(on_reference_heading_updated,
                 void(const location::Update<location::Heading>&));
    MOCK_METHOD1(on_reference_velocity_updated,
                 void(const location::Update<location::Velocity>&));

};

struct NullProviderSelectionPolicy : public location::ProviderSelectionPolicy
{
    location::ProviderSelection
    determine_provider_selection_for_criteria(
            const location::Criteria&,
            const location::ProviderEnumerator&)
    {
        return location::ProviderSelection {
            location::Provider::Ptr{},
            location::Provider::Ptr{},
            location::Provider::Ptr{}};
    }
};
}

TEST(Engine, adding_and_removing_providers_inserts_and_erases_from_underlying_collection)
{
    location::Engine engine {std::make_shared<NullProviderSelectionPolicy>()};

    auto provider1 = std::make_shared<testing::NiceMock<MockProvider>>();
    auto provider2 = std::make_shared<testing::NiceMock<MockProvider>>();

    engine.add_provider(provider1);
    EXPECT_TRUE(engine.has_provider(provider1));

    engine.add_provider(provider2);
    EXPECT_TRUE(engine.has_provider(provider2));

    engine.remove_provider(provider1);
    EXPECT_FALSE(engine.has_provider(provider1));

    engine.remove_provider(provider2);
    EXPECT_FALSE(engine.has_provider(provider2));
}

TEST(Engine, adding_a_null_provider_throws)
{
    location::Engine engine {std::make_shared<NullProviderSelectionPolicy>()};

    EXPECT_ANY_THROW(engine.add_provider(location::Provider::Ptr {}););
}

namespace
{
struct MockProviderSelectionPolicy : public location::ProviderSelectionPolicy
{
    ~MockProviderSelectionPolicy() noexcept
    {
    }

    MOCK_METHOD2(determine_provider_selection_for_criteria,
                 location::ProviderSelection(
                     const location::Criteria&,
                     const location::ProviderEnumerator&));
};
}

TEST(Engine, provider_selection_policy_is_invoked_when_matching_providers_to_criteria)
{
    using namespace ::testing;

    MockProviderSelectionPolicy policy;
    location::Engine engine
    {
        location::ProviderSelectionPolicy::Ptr
        {
            &policy,
            [](location::ProviderSelectionPolicy*) {}
        }
    };

    EXPECT_CALL(policy, determine_provider_selection_for_criteria(_,_))
            .Times(1)
            .WillOnce(Return(location::ProviderSelection {
                        location::Provider::Ptr{},
                        location::Provider::Ptr{},
                        location::Provider::Ptr{}}));

    auto selection = engine.determine_provider_selection_for_criteria(location::Criteria {});
}

TEST(Engine, adding_a_provider_creates_connections_to_engine_configuration_properties)
{
    using namespace ::testing;
    auto provider = std::make_shared<NiceMock<MockProvider>>();
    auto selection_policy = std::make_shared<NiceMock<MockProviderSelectionPolicy>>();
    location::Engine engine{selection_policy};
    engine.add_provider(provider);

    EXPECT_CALL(*provider, on_wifi_and_cell_reporting_state_changed(_)).Times(1);

    EXPECT_CALL(*provider, on_reference_location_updated(_)).Times(1);
    EXPECT_CALL(*provider, on_reference_heading_updated(_)).Times(1);
    EXPECT_CALL(*provider, on_reference_velocity_updated(_)).Times(1);

    engine.configuration.wifi_and_cell_id_reporting_state = location::WifiAndCellIdReportingState::on;
    engine.updates.reference_location = location::Update<location::Position>{};
    engine.updates.reference_heading = location::Update<location::Heading>{};
    engine.updates.reference_velocity = location::Update<location::Velocity>{};
}

TEST(Engine, switching_the_engine_off_results_in_updates_being_stopped)
{
    using namespace ::testing;

    auto provider = std::make_shared<NiceMock<MockProvider>>();
    provider->state_controller()->start_position_updates();
    provider->state_controller()->start_heading_updates();
    provider->state_controller()->start_velocity_updates();

    auto selection_policy = std::make_shared<NiceMock<MockProviderSelectionPolicy>>();
    location::Engine engine{selection_policy};
    engine.add_provider(provider);

    EXPECT_CALL(*provider, stop_position_updates()).Times(1);
    EXPECT_CALL(*provider, stop_velocity_updates()).Times(1);
    EXPECT_CALL(*provider, stop_heading_updates()).Times(1);

    engine.configuration.engine_state = location::Engine::Status::off;
}

TEST(ConnectivityManager, default_implementation_available)
{
    auto manager = location::connectivity::platform_default_manager();

    for (const auto& cell : manager->visible_radio_cells().get())
            std::cout << cell << std::endl;

    EXPECT_NO_THROW(
    {
        for (const auto& wifi: manager->visible_wireless_networks().get())
            std::cout << wifi << std::endl;
    });
}
