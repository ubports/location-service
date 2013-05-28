#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_CONFIGURATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_CONFIGURATION_H_

#include "com/ubuntu/location/service/permission_manager.h"

#include "com/ubuntu/location/engine.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_selection_policy.h"

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class Configuration
{
public:
    virtual ~Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    virtual Engine::Ptr the_engine(
    	const std::set<Provider::Ptr>& provider_set,
        const ProviderSelectionPolicy::Ptr& provider_selection_policy) = 0;
    
    virtual ProviderSelectionPolicy::Ptr the_provider_selection_policy() = 0;
    virtual std::set<Provider::Ptr> the_provider_set() = 0;
    virtual PermissionManager::Ptr the_permission_manager() = 0;

protected:
    Configuration() = default;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_CONFIGURATION_H_
