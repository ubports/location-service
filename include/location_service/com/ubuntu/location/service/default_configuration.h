#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_

#include "com/ubuntu/location/service/configuration.h"

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class DefaultConfiguration
{
public:
    DefaultConfiguration();
    DefaultConfiguration(const DefaultConfiguration&) = delete;
    DefaultConfiguration& operator=(const DefaultConfiguration&) = delete;
    ~DefaultConfiguration() noexcept;

    virtual Engine::Ptr the_engine(
        const std::set<Provider::Ptr>& provider_set,
        const ProviderSelectionPolicy::Ptr& provider_selection_policy);

    ProviderSelectionPolicy::Ptr the_provider_selection_policy();

    std::set<Provider::Ptr> the_provider_set(
        const Provider::Ptr& seed = Provider::Ptr {});

    PermissionManager::Ptr the_permission_manager();
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_CONFIGURATION_H_
