#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_

#include "com/ubuntu/location/service/session/skeleton.h"

#include "com/ubuntu/location/provider.h"

#include <functional>
#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
namespace session
{
class Implementation : public Skeleton
{
  public:
    Implementation(
        const org::freedesktop::dbus::Bus::Ptr& bus, 
        const org::freedesktop::dbus::types::ObjectPath& session_path, 
        const Provider::Ptr& provider)
        : Skeleton(bus, session_path),
          provider(provider),
          controller(provider->state_controller())
    {
        position_updates_connection = 
                provider->subscribe_to_position_updates([this](const Update<Position>& update)
                                                        {
                                                            access_position_updates_channel()(update);
                                                        });
        heading_updates_connection = 
                provider->subscribe_to_heading_updates([this](const Update<Heading>& update)
                                                       {
                                                           access_heading_updates_channel()(update);
                                                       });
        velocity_updates_connection = 
                provider->subscribe_to_velocity_updates([this](const Update<Velocity>& update)
                                                        {
                                                            access_velocity_updates_channel()(update);
                                                        });
    }
    
    virtual ~Implementation() noexcept
    {
    }

    virtual void start_position_updates() 
    {
        controller->start_position_updates();
        if (controller->cached_position_update().is_valid())
            access_position_updates_channel()(controller->cached_position_update().value());
    }

    virtual void stop_position_updates() noexcept 
    {
        controller->stop_position_updates();
    }

    virtual void start_velocity_updates() 
    {
        controller->start_velocity_updates();
        if (controller->cached_velocity_update().is_valid())
            access_velocity_updates_channel()(controller->cached_velocity_update().value());
    }

    virtual void stop_velocity_updates() noexcept
    {
        controller->stop_velocity_updates();
    }

    virtual void start_heading_updates()
    {
        controller->start_heading_updates();
        if (controller->cached_heading_update().is_valid())
            access_heading_updates_channel()(controller->cached_heading_update().value());
    }

    virtual void stop_heading_updates() noexcept
    {
        controller->stop_heading_updates();
    }

  private:
    Provider::Ptr provider;
    Provider::Controller::Ptr controller;
    ScopedChannelConnection position_updates_connection;
    ScopedChannelConnection velocity_updates_connection;
    ScopedChannelConnection heading_updates_connection;

};
}
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_IMPLEMENTATION_H_
