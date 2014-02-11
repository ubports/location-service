#include "com/ubuntu/location/service/session/implementation.h"

#include <functional>
#include <memory>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culss::Implementation::Private
{
    Private(const Provider::Ptr& provider) : provider(provider)
    {
    }

    Provider::Ptr provider;
    ScopedChannelConnection position_updates_connection;
    ScopedChannelConnection velocity_updates_connection;
    ScopedChannelConnection heading_updates_connection;
};

culss::Implementation::Implementation(
    const dbus::Bus::Ptr& bus,
    const dbus::types::ObjectPath& session_path,
    const cul::Provider::Ptr& provider)
        : Skeleton(bus, session_path),
          d(new Private{provider})
{
    if (!provider)
        throw std::runtime_error("Cannot create implementation for null provider.");
    
    d->position_updates_connection =
            provider->subscribe_to_position_updates([this](const Update<Position>& update)
                                                    {
                                                        access_position_updates_channel()(update);
                                                    });
    d->heading_updates_connection =
            provider->subscribe_to_heading_updates([this](const Update<Heading>& update)
                                                   {
                                                       access_heading_updates_channel()(update);
                                                   });
    d->velocity_updates_connection =
            provider->subscribe_to_velocity_updates([this](const Update<Velocity>& update)
                                                    {
                                                        access_velocity_updates_channel()(update);
                                                    });
}

culss::Implementation::~Implementation() noexcept
{
}

void culss::Implementation::start_position_updates()
{
    d->provider->state_controller()->start_position_updates();
}

void culss::Implementation::stop_position_updates() noexcept
{
    d->provider->state_controller()->stop_position_updates();
}

void culss::Implementation::start_velocity_updates()
{
    d->provider->state_controller()->start_velocity_updates();
}

void culss::Implementation::stop_velocity_updates() noexcept
{
    d->provider->state_controller()->stop_velocity_updates();
}

void culss::Implementation::start_heading_updates()
{
    d->provider->state_controller()->start_heading_updates();
}

void culss::Implementation::stop_heading_updates() noexcept
{
    d->provider->state_controller()->stop_heading_updates();
}
