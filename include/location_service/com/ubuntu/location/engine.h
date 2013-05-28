#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_

#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_selection_policy.h"

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
class Engine
{
public:
    typedef std::shared_ptr<Engine> Ptr;

    Engine(const std::set<Provider::Ptr>& initial_providers,
           const ProviderSelectionPolicy::Ptr& provider_selection_policy);
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    virtual ProviderSelection determine_provider_selection_for_criteria(const Criteria& criteria);

    virtual bool has_provider(const Provider::Ptr& provider) noexcept;
    virtual void add_provider(const Provider::Ptr& provider);
    virtual void remove_provider(const Provider::Ptr& provider) noexcept;

private:
    std::set<Provider::Ptr> providers;
    ProviderSelectionPolicy::Ptr provider_selection_policy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
