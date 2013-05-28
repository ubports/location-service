#ifndef LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_
#define LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_

#include "com/ubuntu/location/provider_selection_policy.h"

namespace com
{
namespace ubuntu
{
namespace location
{
class DefaultProviderSelectionPolicy : public ProviderSelectionPolicy
{
public:
    DefaultProviderSelectionPolicy();
    ~DefaultProviderSelectionPolicy() noexcept;
    
    ProviderSelection determine_provider_selection_from_set_for_criteria(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_position_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_heading_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);

    Provider::Ptr determine_velocity_updates_provider(
        const Criteria& criteria,
        const std::set<Provider::Ptr>& providers);
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_DEFAULT_PROVIDER_SELECTION_POLICY_H_
