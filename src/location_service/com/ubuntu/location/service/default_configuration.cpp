#include "com/ubuntu/location/service/default_configuration.h"
#include "com/ubuntu/location/service/default_permission_manager.h"

#include "com/ubuntu/location/default_provider_selection_policy.h"
#include "com/ubuntu/location/default_provider_set.h"

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

culs::DefaultConfiguration::DefaultConfiguration()
{
}

culs::DefaultConfiguration::~DefaultConfiguration() noexcept
{
}

cul::Engine::Ptr culs::DefaultConfiguration::the_engine(
    const std::set<cul::Provider::Ptr>& provider_set,
    const cul::ProviderSelectionPolicy::Ptr& provider_selection_policy)
{
    return Engine::Ptr {new Engine{provider_set, provider_selection_policy}};
}

cul::ProviderSelectionPolicy::Ptr culs::DefaultConfiguration::the_provider_selection_policy()
{
    return ProviderSelectionPolicy::Ptr(new DefaultProviderSelectionPolicy());
}

std::set<cul::Provider::Ptr> culs::DefaultConfiguration::the_provider_set(
    const cul::Provider::Ptr& seed)
{
    return default_provider_set(seed);
}

culs::PermissionManager::Ptr culs::DefaultConfiguration::the_permission_manager()
{
    return DefaultPermissionManager::Ptr(new DefaultPermissionManager());
}

