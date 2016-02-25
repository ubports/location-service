#include <com/ubuntu/location/fusion_provider.h>

#include "fusion_provider_selection_policy.h"

namespace location = com::ubuntu::location;

location::ProviderSelection location::FusionProviderSelectionPolicy::determine_provider_selection_for_criteria(
        const location::Criteria&,
        const location::ProviderEnumerator& enumerator)
{
    // We put all providers in a set.
    std::set<location::Provider::Ptr> bag;
    enumerator.for_each_provider([&bag](const location::Provider::Ptr& provider)
    {
        bag.insert(provider);
    });

    auto fusion_providers = std::make_shared<FusionProvider>(bag);

    return location::ProviderSelection
    {
        fusion_providers, // position
        fusion_providers, // heading
        fusion_providers, // velocity
    };
}
