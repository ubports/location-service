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
#include "com/ubuntu/location/service/session/stub.h"

#include <org/freedesktop/dbus/stub.h>

#include <functional>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;
namespace dbus = org::freedesktop::dbus;

culss::Stub::Stub(
    const dbus::Bus::Ptr& bus,
    const dbus::types::ObjectPath& session_path) 
        : dbus::Stub<culss::Interface>(bus),
          session_path(session_path),
          object(access_service()->add_object_for_path(session_path))
{
    object->install_method_handler<culss::Interface::UpdatePosition>(
        std::bind(&Stub::update_position, this, std::placeholders::_1));
    object->install_method_handler<culss::Interface::UpdateHeading>(
        std::bind(&Stub::update_heading, this, std::placeholders::_1));
    object->install_method_handler<culss::Interface::UpdateVelocity>(
        std::bind(&Stub::update_velocity, this, std::placeholders::_1));
}

const dbus::types::ObjectPath& culss::Stub::path() const
{
    return session_path;
}

void culss::Stub::start_position_updates()
{
	auto result = object->invoke_method_synchronously<Interface::StartPositionUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());
}

void culss::Stub::stop_position_updates() noexcept
{    	
	auto result = object->invoke_method_synchronously<Interface::StopPositionUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());    	
}

void culss::Stub::start_velocity_updates()
{
	auto result = object->invoke_method_synchronously<Interface::StartVelocityUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());
}

void culss::Stub::stop_velocity_updates() noexcept
{
	auto result = object->invoke_method_synchronously<Interface::StopVelocityUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());
}

void culss::Stub::start_heading_updates()
{
	auto result = object->invoke_method_synchronously<Interface::StartHeadingUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());
}

void culss::Stub::stop_heading_updates() noexcept
{
	auto result = object->invoke_method_synchronously<Interface::StopHeadingUpdates,void>();

	if (result.is_error())
        throw std::runtime_error(result.error());
}
    
void culss::Stub::update_heading(DBusMessage* msg)
{        
    auto incoming = dbus::Message::from_raw_message(msg);
    try
    {
        Update<Heading> update; incoming->reader() >> update;
        access_heading_updates_channel()(update);
        access_bus()->send(dbus::Message::make_method_return(msg)->get());
    } catch(const std::runtime_error& e)
    {
        access_bus()->send(dbus::Message::make_error(msg, Interface::Errors::ErrorParsingUpdate::name(), e.what())->get());
    }
}

void culss::Stub::update_position(DBusMessage* msg)
{
    auto incoming = dbus::Message::from_raw_message(msg);
    try
    {
        Update<Position> update; incoming->reader() >> update;
        access_position_updates_channel()(update);
        access_bus()->send(dbus::Message::make_method_return(msg)->get());
    } catch(const std::runtime_error& e)
    {
        access_bus()->send(dbus::Message::make_error(msg, Interface::Errors::ErrorParsingUpdate::name(), e.what())->get());
    }
}

void culss::Stub::update_velocity(DBusMessage* msg)
{
    auto incoming = dbus::Message::from_raw_message(msg);
    try
    {
        Update<Velocity> update; incoming->reader() >> update;
        access_velocity_updates_channel()(update);
        access_bus()->send(dbus::Message::make_method_return(msg)->get());
    } catch(const std::runtime_error& e)
    {
        access_bus()->send(dbus::Message::make_error(msg, Interface::Errors::ErrorParsingUpdate::name(), e.what())->get());
    }
}
