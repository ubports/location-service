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
#include <location/dbus/skeleton/session.h>

#include <location/dbus/codec.h>
#include <location/dbus/session.h>

#include <location/logging.h>

#include <functional>

namespace
{
core::dbus::Message::Ptr the_empty_reply()
{
    return core::dbus::Message::Ptr{};
}
}

location::dbus::skeleton::Session::Session(const location::dbus::skeleton::Session::Configuration& config)
        : configuration(config),
          connections
          {
              configuration.local.impl->updates().position.changed().connect(
                  [this](const location::Update<location::Position>& position)
                  {
                      on_position_changed(position);
                  }),
              configuration.local.impl->updates().heading.changed().connect(
                  [this](const location::Update<location::Heading>& heading)
                  {
                      on_heading_changed(heading);
                  }),
              configuration.local.impl->updates().velocity.changed().connect(
                  [this](const location::Update<location::Velocity>& velocity)
                  {
                      on_velocity_changed(velocity);
                  })
          }
{
    configuration.local.object->install_method_handler<location::dbus::Session::StartPositionUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_start_position_updates(msg);
    });

    configuration.local.object->install_method_handler<location::dbus::Session::StopPositionUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_stop_position_updates(msg);
    });

    configuration.local.object->install_method_handler<location::dbus::Session::StartVelocityUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_start_velocity_updates(msg);
    });

    configuration.local.object->install_method_handler<location::dbus::Session::StopVelocityUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_stop_velocity_updates(msg);
    });

    configuration.local.object->install_method_handler<location::dbus::Session::StartHeadingUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_start_heading_updates(msg);
    });

    configuration.local.object->install_method_handler<location::dbus::Session::StopHeadingUpdates>([this](const core::dbus::Message::Ptr& msg)
    {
        on_stop_heading_updates(msg);
    });
}

location::dbus::skeleton::Session::~Session() noexcept
{
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StartPositionUpdates>();
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StopPositionUpdates>();
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StartVelocityUpdates>();
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StopVelocityUpdates>();
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StartHeadingUpdates>();
    configuration.local.object->uninstall_method_handler<location::dbus::Session::StopHeadingUpdates>();
}

void location::dbus::skeleton::Session::on_start_position_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StartPositionUpdates";

    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = Updates::Status::enabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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

void location::dbus::skeleton::Session::on_stop_position_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StopPositionUpdates";
    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = Updates::Status::disabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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

void location::dbus::skeleton::Session::on_start_heading_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StartHeadingUpdates";
    auto reply = the_empty_reply();

    try
    {
        configuration.local.impl->updates().position_status = Updates::Status::enabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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

void location::dbus::skeleton::Session::on_stop_heading_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StopHeadingUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().heading_status = Updates::Status::disabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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

void location::dbus::skeleton::Session::on_start_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StartVelocityUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().velocity_status = Updates::Status::enabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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

void location::dbus::skeleton::Session::on_stop_velocity_updates(const core::dbus::Message::Ptr& msg)
{
    VLOG(10) << "MethodHandler for location::dbus::Session::StopVelocityUpdates";
    auto reply = the_empty_reply();
    try
    {
        configuration.local.impl->updates().velocity_status = Updates::Status::disabled;
        reply = core::dbus::Message::make_method_return(msg);
    } catch(const std::runtime_error& e)
    {
        // We only provide a generic error message to avoid leaking
        // any sort of private data to unprivileged clients.
        reply = core::dbus::Message::make_error(
                    msg,
                    location::dbus::Session::Errors::ErrorStartingUpdate::name(),
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
void location::dbus::skeleton::Session::on_position_changed(const location::Update<location::Position>& position)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<location::dbus::Session::UpdatePosition, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                VLOG(10) << "Failed to communicate position update to client: " << result.error().print();
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
void location::dbus::skeleton::Session::on_heading_changed(const location::Update<location::Heading>& heading)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<location::dbus::Session::UpdateHeading, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                VLOG(10) << "Failed to communicate heading update to client: " << result.error().print();
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
void location::dbus::skeleton::Session::on_velocity_changed(const location::Update<location::Velocity>& velocity)
{
    VLOG(10) << __PRETTY_FUNCTION__;
    try
    {
        configuration.remote.object->invoke_method_asynchronously_with_callback<location::dbus::Session::UpdateVelocity, void>([](const core::dbus::Result<void>& result)
        {
            if (result.is_error())
            {
                VLOG(10) << "Failed to communicate velocity update to client: " << result.error().print();
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

const core::dbus::types::ObjectPath& location::dbus::skeleton::Session::path() const
{
    return configuration.path;
}

location::Service::Session::Updates& location::dbus::skeleton::Session::updates()
{
    return configuration.local.impl->updates();
}
