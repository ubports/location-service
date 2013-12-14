#include <com/ubuntu/location/provider_selection_policy.h>

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

    EXPECT_EQ(cul::Provider::Ptr{}, 
              policy.determine_position_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(cul::Provider::Ptr{}, 
              policy.determine_heading_updates_provider(cul::Criteria{}, enumerator));
    EXPECT_EQ(cul::Provider::Ptr{}, 
              policy.determine_velocity_updates_provider(cul::Criteria{}, enumerator));
    cul::ProviderSelection empty_selection
    {
        cul::Provider::Ptr{},
        cul::Provider::Ptr{},
        cul::Provider::Ptr{}
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
