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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <com/ubuntu/location/service/session/skeleton.h>

#include "interface_p.h"

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>

#include <functional>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culss::Skeleton::Private
{
    culss::Interface::Ptr instance;
    dbus::Bus::Ptr bus;
    dbus::types::ObjectPath session_path;
    dbus::Object::Ptr object;

    struct
    {
        dbus::Service::Ptr service;
        dbus::Object::Ptr session;
    } remote;
};

culss::Skeleton::Skeleton(
        const culss::Interface::Ptr& instance,
        const dbus::Bus::Ptr& bus,
        const dbus::Service::Ptr& service,
        const dbus::Object::Ptr& session,
        const dbus::types::ObjectPath& session_path)
        : dbus::Skeleton<Interface>{bus},
    d(new Private
      {
          instance,
          bus,
          session_path,
          access_service()->add_object_for_path(session_path),
          { service, session }
      })
{
    d->object->install_method_handler<Interface::StartPositionUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {
        try
        {
            d->instance->updates().position_status = culss::Interface::Updates::Status::enabled;
            auto reply = dbus::Message::make_method_return(msg);
            bus->send(reply);
        } catch(const std::runtime_error& e)
        {
            auto error = core::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
            bus->send(error);
        }
    });

    d->object->install_method_handler<Interface::StopPositionUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {
        d->instance->updates().position_status = culss::Interface::Updates::Status::disabled;
        auto reply = core::dbus::Message::make_method_return(msg);
        bus->send(reply);
    });

    d->object->install_method_handler<Interface::StartVelocityUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {
        try
        {
            d->instance->updates().velocity_status = culss::Interface::Updates::Status::enabled;
            auto reply = dbus::Message::make_method_return(msg);
            bus->send(reply);
        } catch(const std::runtime_error& e)
        {
            auto error = core::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
            bus->send(error);
        }
    });

    d->object->install_method_handler<Interface::StopVelocityUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {
        d->instance->updates().velocity_status = culss::Interface::Updates::Status::disabled;
        auto reply = core::dbus::Message::make_method_return(msg);
        bus->send(reply);
    });

    d->object->install_method_handler<Interface::StartHeadingUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {
        try
        {
            d->instance->updates().heading_status = culss::Interface::Updates::Status::enabled;
            auto reply = dbus::Message::make_method_return(msg);
            bus->send(reply);
        } catch(const std::runtime_error& e)
        {
            auto error = core::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
            bus->send(error);
        }
    });

    d->object->install_method_handler<Interface::StopHeadingUpdates>([this, bus](const dbus::Message::Ptr& msg)
    {

        d->instance->updates().heading_status = culss::Interface::Updates::Status::disabled;
        auto reply = core::dbus::Message::make_method_return(msg);
        bus->send(reply);
    });

    instance->updates().position.changed().connect([this](const cul::Update<cul::Position>& position)
    {
        try
        {
            d->remote.session->invoke_method_synchronously<culs::session::Interface::UpdatePosition, void>(position);
        } catch(const std::runtime_error&)
        {
            // We consider the session to be dead once we hit an exception here.
            // We thus remove it from the central and end its lifetime.
            // on_session_died();
        }
    });

    instance->updates().heading.changed().connect([this](const cul::Update<cul::Heading>& heading)
    {
        try
        {
            d->remote.session->invoke_method_synchronously<culs::session::Interface::UpdateHeading, void>(heading);
        } catch(const std::runtime_error&)
        {
            // We consider the session to be dead once we hit an exception here.
            // We thus remove it from the central and end its lifetime.
            //on_session_died();
        }
    });

    instance->updates().velocity.changed().connect([this](const cul::Update<cul::Velocity>& velocity)
    {
        try
        {
            d->remote.session->invoke_method_synchronously<culs::session::Interface::UpdateVelocity, void>(velocity);
        } catch(const std::runtime_error&)
        {
            // We consider the session to be dead once we hit an exception here.
            // We thus remove it from the central and end its lifetime.
            // on_session_died();
        }
    });
}

culss::Skeleton::~Skeleton() noexcept
{
    d->object->uninstall_method_handler<Interface::StartPositionUpdates>();
    d->object->uninstall_method_handler<Interface::StopPositionUpdates>();
    d->object->uninstall_method_handler<Interface::StartVelocityUpdates>();
    d->object->uninstall_method_handler<Interface::StopVelocityUpdates>();
    d->object->uninstall_method_handler<Interface::StartHeadingUpdates>();
    d->object->uninstall_method_handler<Interface::StopHeadingUpdates>();
}

const dbus::types::ObjectPath& culss::Skeleton::path() const
{
    return d->session_path;
}
