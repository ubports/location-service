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

#include <com/ubuntu/location/logging.h>

#include "interface_p.h"

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/skeleton.h>

#include <functional>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;

namespace dbus = core::dbus;

namespace
{
dbus::Message::Ptr the_empty_reply()
{
    return dbus::Message::Ptr{};
}
}

culss::Skeleton::Skeleton(const culss::Skeleton::Configuration& config)
        : dbus::Skeleton<Interface>{config.local.bus},
          configuration(config),
          object(access_service()->add_object_for_path(configuration.path)),
          connections
          {
              configuration.local.impl->updates().position.changed().connect(
                  [this](const cul::Update<cul::Position>& position)
                  {
                      on_position_changed(position);
                  }),
              configuration.local.impl->updates().heading.changed().connect(
                  [this](const cul::Update<cul::Heading>& heading)
                  {
                      on_heading_changed(heading);
                  }),
              configuration.local.impl->updates().velocity.changed().connect(
                  [this](const cul::Update<cul::Velocity>& velocity)
                  {
                      on_velocity_changed(velocity);
                  })
          }
{
    object->install_method_handler<Interface::StartPositionUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_start_position_updates(msg);
    });

    object->install_method_handler<Interface::StopPositionUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_stop_position_updates(msg);
    });

    object->install_method_handler<Interface::StartVelocityUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_start_velocity_updates(msg);
    });

    object->install_method_handler<Interface::StopVelocityUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_stop_velocity_updates(msg);
    });

    object->install_method_handler<Interface::StartHeadingUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_start_heading_updates(msg);
    });

    object->install_method_handler<Interface::StopHeadingUpdates>([this](const dbus::Message::Ptr& msg)
    {
        on_stop_heading_updates(msg);
    });
}

culss::Skeleton::~Skeleton() noexcept
{
    object->uninstall_method_handler<Interface::StartPositionUpdates>();
    object->uninstall_method_handler<Interface::StopPositionUpdates>();
    object->uninstall_method_handler<Interface::StartVelocityUpdates>();
    object->uninstall_method_handler<Interface::StopVelocityUpdates>();
    object->uninstall_method_handler<Interface::StartHeadingUpdates>();
    object->uninstall_method_handler<Interface::StopHeadingUpdates>();
}

void culss::Skeleton::on_start_position_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StartPositionUpdates";

    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = culss::Interface::Updates::Status::enabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not enable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

void culss::Skeleton::on_stop_position_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StopPositionUpdates";
    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = culss::Interface::Updates::Status::disabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not disable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

void culss::Skeleton::on_start_heading_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StartHeadingUpdates";
    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = culss::Interface::Updates::Status::enabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not enable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

void culss::Skeleton::on_stop_heading_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StopHeadingUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().heading_status = culss::Interface::Updates::Status::disabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not enable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

void culss::Skeleton::on_start_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StartVelocityUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().velocity_status = culss::Interface::Updates::Status::enabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not enable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

void culss::Skeleton::on_stop_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for Interface::StopVelocityUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().velocity_status = culss::Interface::Updates::Status::disabled;
        reply = dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    Interface::Errors::ErrorStartingUpdate::name(),
                    "Could not enable position updates");
        SYSLOG(ERROR) << e.what();
    }

    try
    {
        configuration.local.bus->send(reply);
    } catch(const std::exception& e)
    {
        SYSLOG(ERROR) << e.what();
    }
}

// Invoked whenever the actual session impl. for the session reports a position update.
void culss::Skeleton::on_position_changed(const cul::Update<cul::Position>& position)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<culs::session::Interface::UpdatePosition, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                LOG(INFO) << "Failed to communicate position update to client: " << result.error().print();    
            }
        }, position);
    } catch(const std::exception&)
    {
        // We consider the session to be dead once we hit an exception here.
        // We thus remove it from the central and end its lifetime.
        // on_session_died();
    } catch(...)
    {
    }
}

// Invoked whenever the actual session impl. reports a heading update.
void culss::Skeleton::on_heading_changed(const cul::Update<cul::Heading>& heading)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<culs::session::Interface::UpdateHeading, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                LOG(INFO) << "Failed to communicate position update to client: " << result.error().print();    
            }
        }, heading);
    } catch(const std::exception&)
    {
        // We consider the session to be dead once we hit an exception here.
        // We thus remove it from the central and end its lifetime.
        //on_session_died();
    } catch(...)
    {
    }
}

// Invoked whenever the actual session impl. reports a velocity update.
void culss::Skeleton::on_velocity_changed(const cul::Update<cul::Velocity>& velocity)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<culs::session::Interface::UpdateVelocity, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                LOG(INFO) << "Failed to communicate position update to client: " << result.error().print();    
            }
        }, velocity);
    } catch(const std::exception&)
    {
        // We consider the session to be dead once we hit an exception here.
        // We thus remove it from the central and end its lifetime.
        // on_session_died();
    } catch(...)
    {
    }
}

const dbus::types::ObjectPath& culss::Skeleton::path() const
{
    return configuration.path;
}
