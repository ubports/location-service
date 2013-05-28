#ifndef LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_

#include "com/ubuntu/location/provider.h"

#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria;

struct ProviderSelection
{
    Provider::FeatureFlags to_feature_flags() const
    {
        Provider::FeatureFlags flags;
        flags.set(static_cast<std::size_t>(Provider::Feature::position), static_cast<bool>(position_updates_provider));
        flags.set(static_cast<std::size_t>(Provider::Feature::heading), static_cast<bool>(heading_updates_provider));
        flags.set(static_cast<std::size_t>(Provider::Feature::velocity), static_cast<bool>(velocity_updates_provider));

        return flags;
    }

    Provider::Ptr position_updates_provider;
    Provider::Ptr heading_updates_provider;
    Provider::Ptr velocity_updates_provider;
};

class ProviderSelectionPolicy
{
public:
    typedef std::shared_ptr<ProviderSelectionPolicy> Ptr;

    virtual ~ProviderSelectionPolicy() = default;

    virtual ProviderSelection determine_provider_selection_from_set_for_criteria(const Criteria& criteria, const std::set<Provider::Ptr>& providers) = 0;
protected:
    ProviderSelectionPolicy() = default;
private:
    ProviderSelectionPolicy(const ProviderSelectionPolicy&) = delete;
    ProviderSelectionPolicy& operator=(const ProviderSelectionPolicy&) = delete;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_PROVIDER_SELECTION_POLICY_H_
