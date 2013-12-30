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

#include <com/ubuntu/location/service/session/stub.h>

#include "interface_p.h"

#include <com/ubuntu/location/logging.h>

#include <core/dbus/stub.h>

#include <functional>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

struct culss::Stub::Private
{
    Private(Stub* parent,
            const dbus::types::ObjectPath& path,
            const dbus::Object::Ptr& object,
            const core::Connection& position,
            const core::Connection& velocity,
            const core::Connection& heading)
        : parent(parent),
          session_path(path),
          object(object),
          position(position),
          velocity(velocity),
          heading(heading)
    {
    }

    void update_heading(const dbus::Message::Ptr& msg);
    void update_position(const dbus::Message::Ptr& msg);
    void update_velocity(const dbus::Message::Ptr& msg);

    Stub* parent;
    dbus::types::ObjectPath session_path;
    dbus::Object::Ptr object;
    core::ScopedConnection position;
    core::ScopedConnection velocity;
    core::ScopedConnection heading;

};

culss::Stub::Stub(const dbus::Bus::Ptr& bus,
                  const dbus::types::ObjectPath& session_path)
        : dbus::Stub<culss::Interface>(bus),
        d(new Private(this,
                      session_path,
                      access_service()->add_object_for_path(session_path),
                      updates().position_status.changed().connect([this](const Interface::Updates::Status& status)
                      {
                          switch(status)
                          {
                          case Interface::Updates::Status::enabled: start_position_updates(); break;
                          case Interface::Updates::Status::disabled: stop_position_updates(); break;
                          }
                      }),
                      updates().velocity_status.changed().connect([this](const Interface::Updates::Status& status)
                      {
                          switch(status)
                          {
                          case Interface::Updates::Status::enabled: start_velocity_updates(); break;
                          case Interface::Updates::Status::disabled: stop_velocity_updates(); break;
                          }
                      }),
                      updates().heading_status.changed().connect([this](const Interface::Updates::Status& status)
                      {
                          switch(status)
                          {
                          case Interface::Updates::Status::enabled: start_heading_updates(); break;
                          case Interface::Updates::Status::disabled: stop_heading_updates(); break;
                          }
                      })
                      ))
{
    d->object->install_method_handler<culss::Interface::UpdatePosition>(
        std::bind(&Stub::Private::update_position,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<culss::Interface::UpdateHeading>(
        std::bind(&Stub::Private::update_heading,
                  std::ref(d),
                  std::placeholders::_1));
    d->object->install_method_handler<culss::Interface::UpdateVelocity>(
        std::bind(&Stub::Private::update_velocity,
                  std::ref(d),
                  std::placeholders::_1));
}

culss::Stub::~Stub() noexcept {}

const dbus::types::ObjectPath& culss::Stub::path() const
{
    return d->session_path;
}

void culss::Stub::start_position_updates()
{
    auto result = d->object->invoke_method_synchronously<Interface::StartPositionUpdates,void>();

    if (result.is_error())
        throw std::runtime_error(result.error().print());
}

void culss::Stub::stop_position_updates() noexcept
{
    try
    {
        auto result = d->object->invoke_method_synchronously<Interface::StopPositionUpdates,void>();

        if (result.is_error())
            LOG(WARNING) << result.error();
    } catch(const std::runtime_error& e)
    {
        LOG(WARNING) << e.what();
    }
}

void culss::Stub::start_velocity_updates()
{
    auto result = d->object->invoke_method_synchronously<Interface::StartVelocityUpdates,void>();

    if (result.is_error())
        throw std::runtime_error(result.error().print());
}

void culss::Stub::stop_velocity_updates() noexcept
{
    try {
        auto result = d->object->invoke_method_synchronously<Interface::StopVelocityUpdates,void>();

        if (result.is_error())
            LOG(WARNING) << result.error();
    } catch(const std::runtime_error& e)
    {
        LOG(WARNING) << e.what();
    }
}

void culss::Stub::start_heading_updates()
{
    auto result = d->object->invoke_method_synchronously<Interface::StartHeadingUpdates,void>();

    if (result.is_error())
        throw std::runtime_error(result.error().print());
}

void culss::Stub::stop_heading_updates() noexcept
{
    try {
        auto result = d->object->invoke_method_synchronously<Interface::StopHeadingUpdates,void>();

        if (result.is_error())
            LOG(WARNING) << result.error();
    } catch(const std::runtime_error& e)
    {
        LOG(WARNING) << e.what();
    }
}

void culss::Stub::Private::update_heading(const dbus::Message::Ptr& incoming)
{
    try
    {
        Update<Heading> update; incoming->reader() >> update;
        parent->updates().heading = update;
        parent->access_bus()->send(dbus::Message::make_method_return(incoming));
    } catch(const std::runtime_error& e)
    {
        parent->access_bus()->send(
                    dbus::Message::make_error(
                        incoming,
                        Interface::Errors::ErrorParsingUpdate::name(),
                        e.what()));
    }
}

void culss::Stub::Private::update_position(const dbus::Message::Ptr& incoming)
{
    try
    {
        Update<Position> update; incoming->reader() >> update;
        parent->updates().position = update;
        parent->access_bus()->send(dbus::Message::make_method_return(incoming));
    } catch(const std::runtime_error& e)
    {
        parent->access_bus()->send(
                    dbus::Message::make_error(
                        incoming,
                        Interface::Errors::ErrorParsingUpdate::name(),
                        e.what()));
    }
}

void culss::Stub::Private::update_velocity(const dbus::Message::Ptr& incoming)
{
    try
    {
        Update<Velocity> update; incoming->reader() >> update;
        parent->updates().velocity = update;
        parent->access_bus()->send(dbus::Message::make_method_return(incoming));
    } catch(const std::runtime_error& e)
    {
        parent->access_bus()->send(
                    dbus::Message::make_error(
                        incoming,
                        Interface::Errors::ErrorParsingUpdate::name(),
                        e.what()));
    }
}
