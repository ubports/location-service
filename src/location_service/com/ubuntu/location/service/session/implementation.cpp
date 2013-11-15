#include "com/ubuntu/location/service/session/implementation.h"

#include <functional>
#include <memory>

namespace cu = com::ubuntu;
namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = org::freedesktop::dbus;

struct culss::Implementation::Private
{
    Provider::Ptr provider;
    struct Connections
    {
        Connections(
                const cu::Connection& pu,
                const cu::Connection& vu,
                const cu::Connection& hu)
            : position_updates(pu),
              velocity_updates(vu),
              heading_updates(hu)
        {
        }

        cu::Connection position_updates;
        cu::Connection velocity_updates;
        cu::Connection heading_updates;
    } connections;
};

culss::Implementation::Implementation(
    const dbus::Bus::Ptr& bus,
    const dbus::types::ObjectPath& session_path,
    const cul::Provider::Ptr& provider)
        : Skeleton(bus, session_path),
          d(new Private
            {
                provider,
                Private::Connections
                {
                    provider->updates().position.changed().connect(
                        [this](const Update<Position>& update)
                        {
                            updates().position = update;
                        }),
                    provider->updates().heading.changed().connect(
                        [this](const Update<Heading>& update)
                        {
                            updates().heading = update;
                        }),
                    provider->updates().velocity.changed().connect(
                        [this](const Update<Velocity>& update)
                        {
                            updates().velocity = update;
                        })
                }
            })
{
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
