#ifndef MOCK_ENGINE_H
#define MOCK_ENGINE_H

#include <com/ubuntu/location/engine.h>
#include <com/ubuntu/location/provider.h>

#include <gmock/gmock.h>

struct MockEngine : public com::ubuntu::location::Engine
{
    MockEngine(
            const com::ubuntu::location::ProviderSelectionPolicy::Ptr& provider_selection_policy,
            const com::ubuntu::location::Settings::Ptr& settings)
            : com::ubuntu::location::Engine(provider_selection_policy, settings)
    {
    }

    // has_provider and remove_provider cannot be mocked because they are marked noexcept
    MOCK_METHOD1(determine_provider_selection_for_criteria, com::ubuntu::location::ProviderSelection(const com::ubuntu::location::Criteria&));
    MOCK_METHOD1(add_provider, void(const com::ubuntu::location::Provider::Ptr&));
    MOCK_METHOD1(for_each_provider, void(const std::function<void(const com::ubuntu::location::Provider::Ptr&)>& enumerator));
};

#endif // MOCK_ENGINE_H
