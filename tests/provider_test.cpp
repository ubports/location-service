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
#include <com/ubuntu/location/provider.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;

namespace
{
template<typename T>
cul::Update<T> update_as_of_now(const T& value = T())
{
    return cul::Update<T>{value, cul::Clock::now()};
}

class DummyProvider : public com::ubuntu::location::Provider
{
public:
    DummyProvider(cul::Provider::Features feats = cul::Provider::Features::none,
                  cul::Provider::Requirements requs= cul::Provider::Requirements::none)
        : com::ubuntu::location::Provider(feats, requs)
    {
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Position>& update)
    {
        mutable_updates().position(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Velocity>& update)
    {
        mutable_updates().velocity(update);
    }

    void inject_update(const com::ubuntu::location::Update<com::ubuntu::location::Heading>& update)
    {
        mutable_updates().heading(update);
    }
};
}

TEST(Provider, requirement_flags_passed_at_construction_are_correctly_stored)
{
    cul::Provider::Features features = cul::Provider::Features::none;
    cul::Provider::Requirements requirements =
            cul::Provider::Requirements::cell_network |
            cul::Provider::Requirements::data_network |
            cul::Provider::Requirements::monetary_spending |
            cul::Provider::Requirements::satellites;

    DummyProvider provider(features, requirements);

    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::satellites));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::cell_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::data_network));
    EXPECT_TRUE(provider.requires(com::ubuntu::location::Provider::Requirements::monetary_spending));
}

TEST(Provider, feature_flags_passed_at_construction_are_correctly_stored)
{
    cul::Provider::Features all_features
        = cul::Provider::Features::heading |
          cul::Provider::Features::position |
          cul::Provider::Features::velocity;
    DummyProvider provider(all_features);

    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Features::position));
    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Features::velocity));
    EXPECT_TRUE(provider.supports(com::ubuntu::location::Provider::Features::heading));
}

TEST(Provider, delivering_a_message_invokes_subscribers)
{
    DummyProvider dp;
    bool position_update_triggered {false};
    bool heading_update_triggered {false};
    bool velocity_update_triggered {false};

    auto c1 = dp.updates().position.connect(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Position>&)
        {
            position_update_triggered = true;
        });

    auto c2 = dp.updates().heading.connect(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Heading>&)
        {
            heading_update_triggered = true;
        });

    auto c3 = dp.updates().velocity.connect(
        [&](const com::ubuntu::location::Update<com::ubuntu::location::Velocity>&)
        {
            velocity_update_triggered = true;
        });

    dp.inject_update(update_as_of_now<cul::Position>());
    dp.inject_update(update_as_of_now<cul::Heading>());
    dp.inject_update(update_as_of_now<cul::Velocity>());

    EXPECT_TRUE(position_update_triggered);
    EXPECT_TRUE(heading_update_triggered);
    EXPECT_TRUE(velocity_update_triggered);
}

namespace
{
struct MockProvider : public com::ubuntu::location::Provider
{
    MockProvider() : cul::Provider()
    {
    }

    void inject_update(const cul::Update<cul::Position>& update)
    {
        mutable_updates().position(update);
    }

    void inject_update(const cul::Update<cul::Velocity>& update)
    {
        mutable_updates().velocity(update);
    }

    void inject_update(const cul::Update<cul::Heading>& update)
    {
        mutable_updates().heading(update);
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
    NiceMock<MockProvider> provider;

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

TEST(Provider, initial_state_of_controller_is_enabled)
{
    using namespace ::testing;

    NiceMock<MockProvider> p;
    EXPECT_EQ(cul::Provider::Controller::State::enabled, p.state_controller()->state());
}

TEST(Provider, starting_updates_on_a_disabled_provider_does_nothing)
{
    using namespace ::testing;

    NiceMock<MockProvider> p;
    EXPECT_CALL(p, start_position_updates()).Times(0);
    EXPECT_CALL(p, start_heading_updates()).Times(0);
    EXPECT_CALL(p, start_velocity_updates()).Times(0);

    p.state_controller()->state() = cul::Provider::Controller::State::disabled;

    p.state_controller()->start_position_updates();
    p.state_controller()->start_heading_updates();
    p.state_controller()->start_velocity_updates();
}

TEST(Provider, disabling_a_provider_stops_all_updates)
{
    using namespace ::testing;

    NiceMock<MockProvider> p;
    EXPECT_CALL(p, stop_position_updates()).Times(1);
    EXPECT_CALL(p, stop_heading_updates()).Times(1);
    EXPECT_CALL(p, stop_velocity_updates()).Times(1);

    p.state_controller()->start_position_updates();
    p.state_controller()->start_heading_updates();
    p.state_controller()->start_velocity_updates();

    p.state_controller()->state() = cul::Provider::Controller::State::disabled;
}

#include <com/ubuntu/location/proxy_provider.h>

TEST(ProxyProvider, start_and_stop_of_updates_propagates_to_correct_providers)
{
    using namespace ::testing;
    
    NiceMock<MockProvider> mp1, mp2, mp3;
    EXPECT_CALL(mp1, start_position_updates()).Times(Exactly(1));
    EXPECT_CALL(mp1, stop_position_updates()).Times(Exactly(1));
    EXPECT_CALL(mp2, start_heading_updates()).Times(Exactly(1));
    EXPECT_CALL(mp2, stop_heading_updates()).Times(Exactly(1));
    EXPECT_CALL(mp3, start_velocity_updates()).Times(Exactly(1));
    EXPECT_CALL(mp3, stop_velocity_updates()).Times(Exactly(1));

    cul::Provider::Ptr p1{std::addressof(mp1), [](cul::Provider*){}};
    cul::Provider::Ptr p2{std::addressof(mp2), [](cul::Provider*){}};
    cul::Provider::Ptr p3{std::addressof(mp3), [](cul::Provider*){}};
    
    cul::ProviderSelection selection{p1, p2, p3};

    cul::ProxyProvider pp{selection};

    pp.start_position_updates();
    pp.stop_position_updates();

    pp.start_heading_updates();
    pp.stop_heading_updates();

    pp.start_velocity_updates();
    pp.stop_velocity_updates();
}

struct MockEventConsumer
{
    MOCK_METHOD1(on_new_position, void(const cul::Update<cul::Position>&));
    MOCK_METHOD1(on_new_velocity, void(const cul::Update<cul::Velocity>&));
    MOCK_METHOD1(on_new_heading, void(const cul::Update<cul::Heading>&));
};

TEST(ProxyProvider, update_signals_are_routed_from_correct_providers)
{
    using namespace ::testing;
    
    NiceMock<MockProvider> mp1, mp2, mp3;

    cul::Provider::Ptr p1{std::addressof(mp1), [](cul::Provider*){}};
    cul::Provider::Ptr p2{std::addressof(mp2), [](cul::Provider*){}};
    cul::Provider::Ptr p3{std::addressof(mp3), [](cul::Provider*){}};
    
    cul::ProviderSelection selection{p1, p2, p3};

    cul::ProxyProvider pp{selection};

    NiceMock<MockEventConsumer> mec;
    EXPECT_CALL(mec, on_new_position(_)).Times(1);
    EXPECT_CALL(mec, on_new_velocity(_)).Times(1);
    EXPECT_CALL(mec, on_new_heading(_)).Times(1);

    pp.updates().position.connect([&mec](const cul::Update<cul::Position>& p){mec.on_new_position(p);});
    pp.updates().heading.connect([&mec](const cul::Update<cul::Heading>& h){mec.on_new_heading(h);});
    pp.updates().velocity.connect([&mec](const cul::Update<cul::Velocity>& v){mec.on_new_velocity(v);});

    mp1.inject_update(cul::Update<cul::Position>());
    mp2.inject_update(cul::Update<cul::Heading>());
    mp3.inject_update(cul::Update<cul::Velocity>());
}
