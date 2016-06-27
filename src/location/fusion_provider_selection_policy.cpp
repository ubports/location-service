#include <location/fusion_provider.h>
#include <location/fusion_provider_selection_policy.h>
#include <location/newer_or_more_accurate_update_selector.h>

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

    auto fusion_providers = std::make_shared<location::FusionProvider>(bag, std::make_shared<location::NewerOrMoreAccurateUpdateSelector>());

    return location::ProviderSelection
    {
        fusion_providers, // position
        fusion_providers, // heading
        fusion_providers, // velocity
    };
}
