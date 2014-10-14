#include <com/ubuntu/location/provider_selection_policy.h>

#include "mock_event_consumer.h"
#include "mock_provider.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;

namespace
{
struct ProviderSetEnumerator : public cul::ProviderEnumerator
{
    ProviderSetEnumerator(const std::set<std::shared_ptr<cul::Provider>>& providers)
        : providers(providers)
    {
    }

    void for_each_provider(const std::function<void(const std::shared_ptr<cul::Provider>&)>& f) const
    {
        for (const auto& provider : providers)
            f(provider);
    }

    std::set<std::shared_ptr<cul::Provider>> providers;
};

class DummyProvider : public cul::Provider
{
public:
    DummyProvider(cul::Provider::Features feats = cul::Provider::Features::none,
                  cul::Provider::Requirements requs= cul::Provider::Requirements::none)
        : com::ubuntu::location::Provider(feats, requs)
    {
    }

    MOCK_METHOD1(matches_criteria, bool(const cul::Criteria&));
};

static const cul::Provider::Features all_features
    = cul::Provider::Features::heading | cul::Provider::Features::position | cul::Provider::Features::velocity;

auto timestamp = cul::Clock::now();

// Create reference objects for injecting and validating updates.
cul::Update<cul::Position> reference_position_update
{
    {
        cul::wgs84::Latitude{9. * cul::units::Degrees},
        cul::wgs84::Longitude{53. * cul::units::Degrees},
        cul::wgs84::Altitude{-2. * cul::units::Meters}
    },
    timestamp
};

cul::Update<cul::Velocity> reference_velocity_update
{
    {5. * cul::units::MetersPerSecond},
    timestamp
};

cul::Update<cul::Heading> reference_heading_update
{
    {120. * cul::units::Degrees},
    timestamp
};
}

TEST(ProviderSelection, feature_flags_calculation_works_correctly)
{
    cul::Provider::Ptr provider{new DummyProvider{}};

    cul::ProviderSelection selection{provider, provider, provider};

    EXPECT_EQ(all_features, selection.to_feature_flags());
}

#include <com/ubuntu/location/default_provider_selection_policy.h>

TEST(DefaultProviderSelectionPolicy, if_no_provider_matches_criteria_null_is_returned)
{
    using namespace testing;

    cul::DefaultProviderSelectionPolicy policy;

    NiceMock<DummyProvider> provider1, provider2;
    ON_CALL(provider1, matches_criteria(_)).WillByDefault(Return(false));
    ON_CALL(provider2, matches_criteria(_)).WillByDefault(Return(false));
    
    std::set<cul::Provider::Ptr> providers;
    providers.insert(cul::Provider::Ptr{&provider1, [](cul::Provider*){}});
    providers.insert(cul::Provider::Ptr{&provider2, [](cul::Provider*){}});

    ProviderSetEnumerator enumerator{providers};

    EXPECT_EQ(cul::ProviderSelectionPolicy::null_provider(),
              policy.determine_position_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(cul::ProviderSelectionPolicy::null_provider(),
              policy.determine_heading_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(cul::ProviderSelectionPolicy::null_provider(),
              policy.determine_velocity_updates_provider(cul::Criteria{}, enumerator));

    cul::ProviderSelection empty_selection
    {
        cul::ProviderSelectionPolicy::null_provider(),
        cul::ProviderSelectionPolicy::null_provider(),
        cul::ProviderSelectionPolicy::null_provider()
    };
    EXPECT_EQ(empty_selection,
              policy.determine_provider_selection_for_criteria(cul::Criteria{}, enumerator));
}

TEST(DefaultProviderSelectionPolicy, an_already_running_provider_is_preferred)
{
    using namespace testing;

    cul::DefaultProviderSelectionPolicy policy;

    NiceMock<DummyProvider> provider1, provider2;
    ON_CALL(provider1, matches_criteria(_)).WillByDefault(Return(true));
    ON_CALL(provider2, matches_criteria(_)).WillByDefault(Return(true));
    
    provider1.state_controller()->start_position_updates();
    provider1.state_controller()->start_heading_updates();
    provider1.state_controller()->start_velocity_updates();

    cul::Provider::Ptr p1{&provider1, [](cul::Provider*){}};
    cul::Provider::Ptr p2{&provider2, [](cul::Provider*){}};
    
    std::set<cul::Provider::Ptr> providers{{p1, p2}};
    ProviderSetEnumerator enumerator{providers};

    EXPECT_EQ(p1,
              policy.determine_position_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(p1,
              policy.determine_heading_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(p1,
              policy.determine_velocity_updates_provider(cul::Criteria{}, enumerator));
    cul::ProviderSelection ps{p1, p1, p1};
    EXPECT_EQ(ps,
              policy.determine_provider_selection_for_criteria(cul::Criteria{}, enumerator));
}

#include <com/ubuntu/location/non_selecting_provider_selection_policy.h>

TEST(NonSelectingProviderSelectionPolicy, returns_a_selection_of_providers_that_dispatches_to_all_underlying_providers)
{
    using namespace ::testing;

    auto provider1 = std::make_shared<NiceMock<MockProvider>>();
    auto provider2 = std::make_shared<NiceMock<MockProvider>>();

    MockEventConsumer mec;
    EXPECT_CALL(mec, on_new_position(_)).Times(2);
    EXPECT_CALL(mec, on_new_heading(_)).Times(2);
    EXPECT_CALL(mec, on_new_velocity(_)).Times(2);

    EXPECT_CALL(*provider1, start_position_updates()).Times(1);
    EXPECT_CALL(*provider1, stop_position_updates()).Times(1);

    EXPECT_CALL(*provider1, start_velocity_updates()).Times(1);
    EXPECT_CALL(*provider1, stop_velocity_updates()).Times(1);

    EXPECT_CALL(*provider1, start_heading_updates()).Times(1);
    EXPECT_CALL(*provider1, stop_heading_updates()).Times(1);

    EXPECT_CALL(*provider2, start_position_updates()).Times(1);
    EXPECT_CALL(*provider2, stop_position_updates()).Times(1);

    EXPECT_CALL(*provider2, start_velocity_updates()).Times(1);
    EXPECT_CALL(*provider2, stop_velocity_updates()).Times(1);

    EXPECT_CALL(*provider2, start_heading_updates()).Times(1);
    EXPECT_CALL(*provider2, stop_heading_updates()).Times(1);

    std::set<cul::Provider::Ptr> providers{{provider1, provider2}};

    cul::NonSelectingProviderSelectionPolicy policy;

    auto selection = policy.determine_provider_selection_for_criteria(
                cul::Criteria{},
                ProviderSetEnumerator{providers});

    selection.position_updates_provider->updates().position.connect([&mec](const cul::Update<cul::Position>& update)
    {
        mec.on_new_position(update);
    });

    selection.heading_updates_provider->updates().heading.connect([&mec](const cul::Update<cul::Heading>& update)
    {
        mec.on_new_heading(update);
    });

    selection.velocity_updates_provider->updates().velocity.connect([&mec](const cul::Update<cul::Velocity>& update)
    {
        mec.on_new_velocity(update);
    });

    selection.position_updates_provider->state_controller()->start_position_updates();
    selection.position_updates_provider->state_controller()->stop_position_updates();

    selection.heading_updates_provider->state_controller()->start_heading_updates();
    selection.heading_updates_provider->state_controller()->stop_heading_updates();

    selection.velocity_updates_provider->state_controller()->start_velocity_updates();
    selection.velocity_updates_provider->state_controller()->stop_velocity_updates();

    provider1->inject_update(reference_position_update);
    provider2->inject_update(reference_position_update);

    provider1->inject_update(reference_heading_update);
    provider2->inject_update(reference_heading_update);

    provider1->inject_update(reference_velocity_update);
    provider2->inject_update(reference_velocity_update);
}
