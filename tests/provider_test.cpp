#include "com/ubuntu/location/provider.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
class DummyProvider : public com::ubuntu::location::Provider
{
public:
    DummyProvider(com::ubuntu::location::Provider::FeatureFlags feature_flags = com::ubuntu::location::Provider::FeatureFlags {},
                  com::ubuntu::location::Provider::RequirementFlags requirement_flags = com::ubuntu::location::Provider::RequirementFlags {})
        : com::ubuntu::location::Provider(feature_flags, requirement_flags)
    {
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Position>& update)
    {
        deliver_position_updates(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Velocity>& update)
    {
        deliver_velocity_updates(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Heading>& update)
    {
        deliver_heading_updates(update);
    }
};
}

TEST(Provider, requirement_flags_passed_at_construction_are_correctly_stored)
{
    com::ubuntu::location::Provider::FeatureFlags feature_flags;
    com::ubuntu::location::Provider::RequirementFlags requirement_flags;
    requirement_flags.set();
    DummyProvider provider(feature_flags, requirement_flags);

    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirement::satellites));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirement::cell_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirement::data_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirement::monetary_spending));
}

TEST(Provider, feature_flags_passed_at_construction_are_correctly_stored)
{
    com::ubuntu::location::Provider::FeatureFlags flags;
    flags.set();
    DummyProvider provider(flags);

    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Feature::position));
    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Feature::velocity));
    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Feature::heading));
}

TEST(Provider, delivering_a_message_invokes_subscribers)
{
    DummyProvider dp;
    bool position_update_triggered {false};
    bool heading_update_triggered {false};
    bool velocity_update_triggered {false};

    dp.subscribe_to_position_updates([&](const com::ubuntu::location::Update<com::ubuntu::location::Position>&)
    {
        position_update_triggered = true;
    });

    dp.subscribe_to_heading_updates([&](const com::ubuntu::location::Update<com::ubuntu::location::Heading>&)
    {
        heading_update_triggered = true;
    });

    dp.subscribe_to_velocity_updates([&](const com::ubuntu::location::Update<com::ubuntu::location::Velocity>&)
    {
        velocity_update_triggered = true;
    });

    dp.inject_update(com::ubuntu::location::Update<com::ubuntu::location::Position>());
    dp.inject_update(com::ubuntu::location::Update<com::ubuntu::location::Heading>());
    dp.inject_update(com::ubuntu::location::Update<com::ubuntu::location::Velocity>());

    EXPECT_TRUE(position_update_triggered);
    EXPECT_TRUE(heading_update_triggered);
    EXPECT_TRUE(velocity_update_triggered);
}

namespace
{
struct MockProvider : public com::ubuntu::location::Provider
{
    MockProvider()
    {
    }

    MOCK_METHOD0(start_position_updates, void());
    MOCK_METHOD0(stop_position_updates, void());
    MOCK_METHOD0(start_heading_updates, void());
    MOCK_METHOD0(stop_heading_updates, void());
    MOCK_METHOD0(start_velocity_updates, void());
    MOCK_METHOD0(stop_velocity_updates, void());
};
}

TEST(Provider, starting_and_stopping_multiple_times_results_in_exactly_one_call_to_start_and_stop_on_provider)
{
    using namespace ::testing;
    MockProvider provider;

    EXPECT_CALL(provider, start_position_updates()).Times(Exactly(1));
    EXPECT_CALL(provider, stop_position_updates()).Times(Exactly(1));
    EXPECT_CALL(provider, start_heading_updates()).Times(Exactly(1));
    EXPECT_CALL(provider, stop_heading_updates()).Times(Exactly(1));
    EXPECT_CALL(provider, start_velocity_updates()).Times(Exactly(1));
    EXPECT_CALL(provider, stop_velocity_updates()).Times(Exactly(1));

    provider.state_controller()->start_position_updates();
    provider.state_controller()->start_position_updates();
    EXPECT_TRUE(provider.state_controller()->are_position_updates_running());
    provider.state_controller()->stop_position_updates();
    provider.state_controller()->stop_position_updates();
    EXPECT_FALSE(provider.state_controller()->are_position_updates_running());

    provider.state_controller()->start_heading_updates();
    provider.state_controller()->start_heading_updates();
    EXPECT_TRUE(provider.state_controller()->are_heading_updates_running());
    provider.state_controller()->stop_heading_updates();
    provider.state_controller()->stop_heading_updates();
    EXPECT_FALSE(provider.state_controller()->are_heading_updates_running());

    provider.state_controller()->start_velocity_updates();
    provider.state_controller()->start_velocity_updates();
    EXPECT_TRUE(provider.state_controller()->are_velocity_updates_running());
    provider.state_controller()->stop_velocity_updates();
    provider.state_controller()->stop_velocity_updates();
    EXPECT_FALSE(provider.state_controller()->are_velocity_updates_running());
}
