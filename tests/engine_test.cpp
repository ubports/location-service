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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cuc = com::ubuntu::connectivity;
namespace cul = com::ubuntu::location;

namespace
{
struct DummyProvider : public com::ubuntu::location::Provider
{
    DummyProvider() : cul::Provider()
    {
    }
};

struct NullProviderSelectionPolicy : public com::ubuntu::location::ProviderSelectionPolicy
{
    com::ubuntu::location::ProviderSelection
    determine_provider_selection_from_set_for_criteria(const com::ubuntu::location::Criteria&, const std::set<com::ubuntu::location::Provider::Ptr>&)
    {
        return com::ubuntu::location::ProviderSelection {
            com::ubuntu::location::Provider::Ptr{}, 
            com::ubuntu::location::Provider::Ptr{}, 
            com::ubuntu::location::Provider::Ptr{}};
    }
};
}

TEST(Engine, adding_and_removing_providers_inserts_and_erases_from_underlying_collection)
{
    com::ubuntu::location::Engine engine {std::set<com::ubuntu::location::Provider::Ptr>{}, std::make_shared<NullProviderSelectionPolicy>()};

    auto provider1 = std::make_shared<DummyProvider>();
    auto provider2 = std::make_shared<DummyProvider>();

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
    com::ubuntu::location::Engine engine {std::set<com::ubuntu::location::Provider::Ptr>{}, std::make_shared<NullProviderSelectionPolicy>()};

    EXPECT_ANY_THROW(engine.add_provider(com::ubuntu::location::Provider::Ptr {}););
}

namespace
{
struct MockProviderSelectionPolicy : public com::ubuntu::location::ProviderSelectionPolicy
{
    ~MockProviderSelectionPolicy() noexcept
    {
    }

    MOCK_METHOD2(determine_provider_selection_from_set_for_criteria,
                 com::ubuntu::location::ProviderSelection(const com::ubuntu::location::Criteria&, const std::set<com::ubuntu::location::Provider::Ptr>&));
};
}

TEST(Engine, provider_selection_policy_is_invoked_when_matching_providers_to_criteria)
{
    using namespace ::testing;

    MockProviderSelectionPolicy policy;
    com::ubuntu::location::Engine engine
    {
        std::set<com::ubuntu::location::Provider::Ptr>{},
                com::ubuntu::location::ProviderSelectionPolicy::Ptr{&policy, [](com::ubuntu::location::ProviderSelectionPolicy*) {}}
    };

    EXPECT_CALL(policy, determine_provider_selection_from_set_for_criteria(_,_))
            .Times(1)
            .WillOnce(Return(com::ubuntu::location::ProviderSelection {
                        com::ubuntu::location::Provider::Ptr{}, 
                        com::ubuntu::location::Provider::Ptr{}, 
                        com::ubuntu::location::Provider::Ptr{}}));

    auto selection = engine.determine_provider_selection_for_criteria(com::ubuntu::location::Criteria {});
}
