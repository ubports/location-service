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
#include "com/ubuntu/location/service/session/skeleton.h"

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
    void handle_start_position_updates(const core::dbus::Message::Ptr& msg);
    void handle_stop_position_updates(const core::dbus::Message::Ptr& msg);

    void handle_start_velocity_updates(const core::dbus::Message::Ptr& msg);
    void handle_stop_velocity_updates(const core::dbus::Message::Ptr& msg);

    void handle_start_heading_updates(const core::dbus::Message::Ptr& msg);
    void handle_stop_heading_updates(const core::dbus::Message::Ptr& msg);

    Skeleton* parent;
    dbus::Bus::Ptr bus;
    dbus::types::ObjectPath session_path;
    dbus::Object::Ptr object;
};

culss::Skeleton::Skeleton(
    const dbus::Bus::Ptr& bus,
    const dbus::types::ObjectPath& session_path)
        : dbus::Skeleton<Interface>{bus},
    d(new Private
      {
          this,
          bus,
          session_path,
          access_service()->add_object_for_path(session_path)
      })
{
    d->object->install_method_handler<Interface::StartPositionUpdates>(
        std::bind(&Skeleton::Private::handle_start_position_updates,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<Interface::StopPositionUpdates>(
        std::bind(&Skeleton::Private::handle_stop_position_updates,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<Interface::StartVelocityUpdates>(
        std::bind(&Skeleton::Private::handle_start_velocity_updates,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<Interface::StopVelocityUpdates>(
        std::bind(&Skeleton::Private::handle_stop_velocity_updates,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<Interface::StartHeadingUpdates>(
        std::bind(&Skeleton::Private::handle_start_heading_updates,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<Interface::StopHeadingUpdates>(
        std::bind(&Skeleton::Private::handle_stop_heading_updates,
                  std::ref(d),
                  std::placeholders::_1));
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

void culss::Skeleton::Private::handle_start_position_updates(const core::dbus::Message::Ptr& msg)
{
    try
    {
        parent->start_position_updates();
        auto reply = dbus::Message::make_method_return(msg);
        bus->send(reply);
    } catch(const std::runtime_error& e)
    {
        auto error = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    e.what());

        bus->send(error);
    }
}

void culss::Skeleton::Private::handle_stop_position_updates(const core::dbus::Message::Ptr& msg)
{
    parent->stop_position_updates();
    auto reply = core::dbus::Message::make_method_return(msg);
    bus->send(reply);
}

void culss::Skeleton::Private::handle_start_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    try
    {
        parent->start_velocity_updates();
        auto reply = core::dbus::Message::make_method_return(msg);
        bus->send(reply);
    } catch(const std::runtime_error& e)
    {
        auto error = core::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
        bus->send(error);
    }
}

void culss::Skeleton::Private::handle_stop_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    parent->stop_velocity_updates();
    auto reply = core::dbus::Message::make_method_return(msg);
    bus->send(reply);
}

void culss::Skeleton::Private::handle_start_heading_updates(const core::dbus::Message::Ptr& msg)
{
    try
    {
        parent->start_heading_updates();
        auto reply = core::dbus::Message::make_method_return(msg);
        bus->send(reply);
    } catch(const std::runtime_error& e)
    {
        auto error = core::dbus::Message::make_error(msg, Interface::Errors::ErrorStartingUpdate::name(), e.what());
        bus->send(error);
    }
}

void culss::Skeleton::Private::handle_stop_heading_updates(const core::dbus::Message::Ptr& msg)
{
    parent->stop_heading_updates();
    auto reply = core::dbus::Message::make_method_return(msg);
    bus->send(reply);
}
