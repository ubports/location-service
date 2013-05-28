#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROXY_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROXY_PROVIDER_H_

#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_selection_policy.h"

#include <bitset>
#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
class ProxyProvider : public Provider
{
public:
    typedef std::shared_ptr<ProxyProvider> Ptr;

    ProxyProvider(const ProviderSelection& selection);
    ~ProxyProvider() noexcept;

    ChannelConnection subscribe_to_position_updates(std::function<void(const Update<Position>&)> f);
    ChannelConnection subscribe_to_heading_updates(std::function<void(const Update<Heading>&)> f);
    ChannelConnection subscribe_to_velocity_updates(std::function<void(const Update<Velocity>&)> f);

    virtual void start_position_updates();
    virtual void stop_position_updates();

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates();

    virtual void start_heading_updates();
    virtual void stop_heading_updates();
    
private:
    Provider::Ptr position_updates_provider;
    Provider::Ptr heading_updates_provider;
    Provider::Ptr velocity_updates_provider;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROXY_PROVIDER_H_
