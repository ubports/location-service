/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Manuel de la Peña <manuel.delapena@canonical.com>
 */

#include <location/providers/remote/provider.h>
#include <location/providers/remote/interface.h>

#include <location/dbus/codec.h>
#include <location/events/registry.h>
#include <location/events/reference_position_updated.h>
#include <location/events/wifi_and_cell_id_reporting_state_changed.h>
#include <location/logging.h>

#include <core/dbus/object.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/signal.h>
#include <core/dbus/asio/executor.h>

#include <core/posix/this_process.h>

#include <boost/asio.hpp>

#include <thread>

namespace cul = location;
namespace remote = location::providers::remote;

namespace dbus = core::dbus;

namespace
{
template<typename T>
T throw_if_error_or_return(const dbus::Result<T>& result)
{
    if (result.is_error()) throw std::runtime_error
    {
        result.error().print()
    };
    return result.value();
}

core::dbus::types::ObjectPath generate_path_for_observer()
{
    static constexpr const char* pattern{"/com/ubuntu/location/provider/observer/%1%"};
    static std::size_t counter{0};

    return core::dbus::types::ObjectPath{(boost::format{pattern} % ++counter).str()};
}
}

remote::Provider::Observer::Stub::Stub(const core::dbus::Object::Ptr& object)
    : object{object}
{

}

void remote::Provider::Observer::Stub::on_new_position(const Update<Position>& update)
{
    try
    {
        object->invoke_method_asynchronously_with_callback<remote::Interface::Observer::UpdatePosition, void>([](const core::dbus::Result<void>&)
        {

        }, update);
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << e.what();
    }
    catch(...)
    {
    }
}

void remote::Provider::Observer::Stub::on_new_heading(const Update<Heading>& update)
{
    try
    {
        object->invoke_method_asynchronously_with_callback<remote::Interface::Observer::UpdateHeading, void>([](const core::dbus::Result<void>&)
        {

        }, update);
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << e.what();
    }
    catch(...)
    {
    }
}

void remote::Provider::Observer::Stub::on_new_velocity(const Update<Velocity>& update)
{
    try
    {
        object->invoke_method_asynchronously_with_callback<remote::Interface::Observer::UpdateVelocity, void>([](const core::dbus::Result<void>&)
        {

        }, update);
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << e.what();
    }
    catch(...)
    {
    }
}

remote::Provider::Observer::Skeleton::Skeleton(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object, const std::shared_ptr<Observer>& impl)
    : bus{bus}, object{object}, impl{impl}
{
    object->install_method_handler<remote::Interface::Observer::UpdatePosition>([this](const core::dbus::Message::Ptr& msg)
    {
        VLOG(50) << "remote::Interface::Observer::UpdatePosition";
        location::Update<location::Position> update; msg->reader() >> update;
        on_new_position(update);
        Skeleton::bus->send(dbus::Message::make_method_return(msg));
    });

    object->install_method_handler<remote::Interface::Observer::UpdateHeading>([this](const core::dbus::Message::Ptr& msg)
    {
        VLOG(50) << "remote::Interface::Observer::UpdateHeading";
        location::Update<location::Heading> update; msg->reader() >> update;
        on_new_heading(update);
        Skeleton::bus->send(dbus::Message::make_method_return(msg));
    });

    object->install_method_handler<remote::Interface::Observer::UpdateVelocity>([this](const core::dbus::Message::Ptr& msg)
    {
        VLOG(50) << "remote::Interface::Observer::UpdateVelocity";
        location::Update<location::Velocity> update; msg->reader() >> update;
        on_new_velocity(update);
        Skeleton::bus->send(dbus::Message::make_method_return(msg));
    });
}

void remote::Provider::Observer::Skeleton::on_new_position(const Update<Position>& update)
{
    impl->on_new_position(update);
}

void remote::Provider::Observer::Skeleton::on_new_heading(const Update<Heading>& update)
{
    impl->on_new_heading(update);
}

void remote::Provider::Observer::Skeleton::on_new_velocity(const Update<Velocity>& update)
{
    impl->on_new_velocity(update);
}

struct remote::Provider::Stub::Private
{
    Private(const remote::stub::Configuration& config, location::Provider::Requirements requirements)
            : bus{config.bus},
              service{config.service},
              object{config.object},
              requirements{requirements},
              stub{object}
    {
    }

    ~Private()
    {
    }

    dbus::Bus::Ptr bus;
    dbus::Service::Ptr service;
    dbus::Object::Ptr object;
    location::Provider::Requirements requirements;
    remote::Interface::Stub stub;
    std::set<std::shared_ptr<remote::Provider::Observer>> observers;
    struct
    {
        core::Signal<location::Update<location::Position>> position;
        core::Signal<location::Update<location::Heading>> heading;
        core::Signal<location::Update<location::Velocity>> velocity;
    } updates;
};

void remote::Provider::Stub::create_instance_with_config(const remote::stub::Configuration& config, const std::function<void(const Provider::Ptr&)>& cb)
{
    config.object->invoke_method_asynchronously_with_callback<remote::Interface::Requirements, Provider::Requirements>([config, cb](const core::dbus::Result<Provider::Requirements>& result)
    {
        VLOG(50) << "Finished querying results from remote provider: " << std::boolalpha << result.is_error();
        if (not result.is_error())
        {
            std::shared_ptr<remote::Provider::Stub> provider{new remote::Provider::Stub{config, result.value()}};
            cb(provider->finalize());
        }
        else
        {
            LOG(WARNING) << result.error().print();
        }
    });
}

remote::Provider::Stub::Stub(const remote::stub::Configuration& config, Provider::Requirements requirements)
        : d(new Private(config, requirements))
{
}

std::shared_ptr<remote::Provider::Stub> remote::Provider::Stub::finalize()
{
    auto thiz = shared_from_this();
    add_observer(thiz);
    return thiz;
}

remote::Provider::Stub::~Stub() noexcept
{
    VLOG(10) << __PRETTY_FUNCTION__;
}

void remote::Provider::Stub::add_observer(const std::shared_ptr<Observer>& observer)
{
    auto thiz = shared_from_this();

    auto path = generate_path_for_observer();
    auto skeleton = std::make_shared<remote::Provider::Observer::Skeleton>(
                d->bus, d->service->add_object_for_path(path), observer);

    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::AddObserver, void>([thiz, skeleton](const dbus::Result<void>& result)
    {
        if (not result.is_error())
            thiz->d->observers.insert(skeleton);
        else
            LOG(WARNING) << result.error().print();
    }, path);
}

void remote::Provider::Stub::on_new_position(const Update<Position>& update)
{
    d->updates.position(update);
}

void remote::Provider::Stub::on_new_heading(const Update<Heading>& update)
{
    d->updates.heading(update);
}

void remote::Provider::Stub::on_new_velocity(const Update<Velocity>& update)
{
    d->updates.velocity(update);
}

void remote::Provider::Stub::on_new_event(const Event& event)
{
    VLOG(50) << __PRETTY_FUNCTION__;
    events::All all;

    if (event.type() == TypeOf<events::ReferencePositionUpdated>::query())
        all = dynamic_cast<const events::ReferencePositionUpdated&>(event);
    else if (event.type() == TypeOf<events::WifiAndCellIdReportingStateChanged>::query())
        all = dynamic_cast<const events::WifiAndCellIdReportingStateChanged&>(event);

    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::OnNewEvent, void>([](const dbus::Result<void>& result)
    {
        if (result.is_error())
            LOG(WARNING) << result.error().print();
    }, all);
}

void remote::Provider::Stub::enable()
{
    VLOG(50) << __PRETTY_FUNCTION__;
    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::Enable, void>([](const dbus::Result<void>& result)
    {
        if (result.is_error()) LOG(WARNING) << result.error().print();
    });
}

void remote::Provider::Stub::disable()
{
    VLOG(50) << __PRETTY_FUNCTION__;
    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::Disable, void>([](const dbus::Result<void>& result)
    {
        if (result.is_error()) LOG(WARNING) << result.error().print();
    });
}

void remote::Provider::Stub::activate()
{
    VLOG(50) << __PRETTY_FUNCTION__;
    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::Activate, void>([](const dbus::Result<void>& result)
    {
        if (result.is_error()) LOG(WARNING) << result.error().print();
    });
}

void remote::Provider::Stub::deactivate()
{
    VLOG(50) << __PRETTY_FUNCTION__;
    d->stub.object->invoke_method_asynchronously_with_callback<remote::Interface::Deactivate, void>([](const dbus::Result<void>& result)
    {
        if (result.is_error()) LOG(WARNING) << result.error().print();
    });
}

location::Provider::Requirements remote::Provider::Stub::requirements() const
{
    return d->requirements;
}

bool remote::Provider::Stub::satisfies(const Criteria& criteria)
{
    return throw_if_error_or_return(d->stub.object->transact_method<remote::Interface::Satisfies, bool>(criteria));
}

const core::Signal<location::Update<location::Position>>& remote::Provider::Stub::position_updates() const
{
    return d->updates.position;
}

const core::Signal<location::Update<location::Heading>>& remote::Provider::Stub::heading_updates() const
{
    return d->updates.heading;
}

const core::Signal<location::Update<location::Velocity>>& remote::Provider::Stub::velocity_updates() const
{
    return d->updates.velocity;
}

struct remote::Provider::Skeleton::Private
{
    Private(const remote::skeleton::Configuration& config)
            : bus(config.bus),
              skeleton(config.object),
              impl(config.provider),
              connections
              {
                  impl->position_updates().connect([this](const cul::Update<cul::Position>& position)
                  {
                      VLOG(100) << "Position changed reported by impl: " << position;                      
                      for (const auto& observer : observers)
                          observer->on_new_position(position);
                  }),
                  impl->heading_updates().connect([this](const cul::Update<cul::Heading>& heading)
                  {
                      VLOG(100) << "Heading changed reported by impl: " << heading;
                      for (const auto& observer : observers)
                          observer->on_new_heading(heading);
                  }),
                  impl->velocity_updates().connect([this](const cul::Update<cul::Velocity>& velocity)
                  {
                      VLOG(100) << "Velocity changed reported by impl: " << velocity;
                      for (const auto& observer : observers)
                          observer->on_new_velocity(velocity);
                  })
              }
    {
    }

    core::dbus::Bus::Ptr bus;
    remote::Interface::Skeleton skeleton;
    cul::Provider::Ptr impl;
    std::set<std::shared_ptr<remote::Provider::Observer>> observers;
    // All connections to signals go here.
    struct
    {
        core::ScopedConnection position_changed;
        core::ScopedConnection heading_changed;
        core::ScopedConnection velocity_changed;
    } connections;
};

remote::Provider::Skeleton::Skeleton(const remote::skeleton::Configuration& config)
    : cul::Provider(),
      d(new Private(config))
{
    // And install method handlers.
    d->skeleton.object->install_method_handler<remote::Interface::Satisfies>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Satisfies";

        cul::Criteria criteria; msg->reader() >> criteria;
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << satisfies(criteria);

        d->bus->send(reply);
    });

    d->skeleton.object->install_method_handler<remote::Interface::Requirements>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Requirements";

        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << requirements();

        d->bus->send(reply);
    });

    d->skeleton.object->install_method_handler<remote::Interface::Enable>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Enable";
        enable();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::Disable>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Disable";
        disable();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::Activate>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Activate";
        activate();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::Deactivate>([this](const dbus::Message::Ptr & msg)
    {
        VLOG(1) << "Deactivate";
        deactivate();
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::AddObserver>([this](const dbus::Message::Ptr& msg)
    {
        VLOG(1) << "AddObserver";
        core::dbus::types::ObjectPath path; msg->reader() >> path;
        auto object = dbus::Service::use_service(d->bus, msg->sender())->object_for_path(path);
        add_observer(std::make_shared<remote::Provider::Observer::Stub>(object));
        d->bus->send(dbus::Message::make_method_return(msg));
    });

    d->skeleton.object->install_method_handler<remote::Interface::OnNewEvent>([this](const dbus::Message::Ptr& msg)
    {
        VLOG(1) << "OnNewEvent";
        events::All all; msg->reader() >> all;
        switch (all.which())
        {
        case 1: on_new_event(boost::get<events::ReferencePositionUpdated>(all));            break;
        case 2: on_new_event(boost::get<events::WifiAndCellIdReportingStateChanged>(all));  break;
        }
        d->bus->send(dbus::Message::make_method_return(msg));
    });
}

remote::Provider::Skeleton::~Skeleton() noexcept
{
    d->skeleton.object->uninstall_method_handler<remote::Interface::Satisfies>();

    d->skeleton.object->uninstall_method_handler<remote::Interface::Requirements>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::Enable>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::Disable>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::Activate>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::Deactivate>();
    d->skeleton.object->uninstall_method_handler<remote::Interface::AddObserver>();
}

void remote::Provider::Skeleton::add_observer(const std::shared_ptr<Observer>& observer)
{
    d->observers.insert(observer);
}

// We just forward calls to the actual implementation
bool remote::Provider::Skeleton::satisfies(const cul::Criteria& criteria)
{
    return d->impl->satisfies(criteria);
}

location::Provider::Requirements remote::Provider::Skeleton::requirements() const
{
    return d->impl->requirements();
}

void remote::Provider::Skeleton::on_new_event(const Event& e)
{
    d->impl->on_new_event(e);
}

void remote::Provider::Skeleton::enable()
{
    d->impl->enable();
}

void remote::Provider::Skeleton::disable()
{
    d->impl->disable();
}

void remote::Provider::Skeleton::activate()
{
    d->impl->activate();
}

void remote::Provider::Skeleton::deactivate()
{
    d->impl->deactivate();
}

const core::Signal<location::Update<location::Position>>& remote::Provider::Skeleton::position_updates() const
{
    return d->impl->position_updates();
}

const core::Signal<location::Update<location::Heading>>& remote::Provider::Skeleton::heading_updates() const
{
    return d->impl->heading_updates();
}

const core::Signal<location::Update<location::Velocity>>& remote::Provider::Skeleton::velocity_updates() const
{
    return d->impl->velocity_updates();
}
