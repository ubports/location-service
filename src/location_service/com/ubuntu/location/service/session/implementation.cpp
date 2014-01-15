#include <com/ubuntu/location/service/session/implementation.h>
#include <com/ubuntu/location/logging.h>

#include <functional>
#include <memory>

namespace cu = com::ubuntu;
namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culss::Implementation::Private
{
    Provider::Ptr provider;
    struct Connections
    {
        Connections(
                const core::Connection& pu,
                const core::Connection& vu,
                const core::Connection& hu)
            : position_updates(pu),
              velocity_updates(vu),
              heading_updates(hu)
        {
        }

        core::ScopedConnection position_updates;
        core::ScopedConnection velocity_updates;
        core::ScopedConnection heading_updates;
    } connections;
};

culss::Implementation::Implementation(const cul::Provider::Ptr& provider)
        : Interface(),
          d(new Private
            {
                provider,
                Private::Connections
                {
                    provider->updates().position.connect(
                        [this](const Update<Position>& update)
                        {
                            updates().position = update;
                        }),
                    provider->updates().heading.connect(
                        [this](const Update<Heading>& update)
                        {
                            updates().heading = update;
                        }),
                    provider->updates().velocity.connect(
                        [this](const Update<Velocity>& update)
                        {
                            updates().velocity = update;
                        })
                }
            })
{
    updates().position_status.changed().connect([this](const Interface::Updates::Status& status)
    {
        switch(status)
        {
        case Interface::Updates::Status::enabled: start_position_updates(); break;
        case Interface::Updates::Status::disabled: stop_position_updates(); break;
        }
    });
    updates().velocity_status.changed().connect([this](const Interface::Updates::Status& status)
    {
        switch(status)
        {
        case Interface::Updates::Status::enabled: start_velocity_updates(); break;
        case Interface::Updates::Status::disabled: stop_velocity_updates(); break;
        }
    });
    updates().heading_status.changed().connect([this](const Interface::Updates::Status& status)
    {
        switch(status)
        {
        case Interface::Updates::Status::enabled: start_heading_updates(); break;
        case Interface::Updates::Status::disabled: stop_heading_updates(); break;
        }
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
