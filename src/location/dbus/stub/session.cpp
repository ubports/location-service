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

#include <location/dbus/stub/session.h>
#include <location/dbus/codec.h>
#include <location/dbus/session.h>

#include <location/logging.h>

location::dbus::stub::Session::Session(const core::dbus::Bus::Ptr& connection,
                                       const core::dbus::Object::Ptr& object)
        : connection_{connection},
          object_{object},
          position_{updates_.position_status.changed().connect([this](const Updates::Status& status)
          {
              switch(status)
              {
              case Updates::Status::enabled: start_position_updates(); break;
              case Updates::Status::disabled: stop_position_updates(); break;
              }
          })},
          velocity_{updates_.velocity_status.changed().connect([this](const Updates::Status& status)
          {
              switch(status)
              {
              case Updates::Status::enabled: start_velocity_updates(); break;
              case Updates::Status::disabled: stop_velocity_updates(); break;
              }
          })},
          heading_{updates_.heading_status.changed().connect([this](const Updates::Status& status)
          {
              switch(status)
              {
              case Updates::Status::enabled: start_heading_updates(); break;
              case Updates::Status::disabled: stop_heading_updates(); break;
              }
          })}
{
    object_->install_method_handler<location::dbus::Session::UpdatePosition>([this](const core::dbus::Message::Ptr& incoming)
    {
        VLOG(10) << __PRETTY_FUNCTION__;

        try
        {
            Update<Position> update; incoming->reader() >> update;
            updates_.position = update;
            connection_->send(core::dbus::Message::make_method_return(incoming));
        } catch(const std::runtime_error& e)
        {
            connection_->send(core::dbus::Message::make_error(
                                  incoming,
                                  location::dbus::Session::Errors::ErrorParsingUpdate::name(),
                                  e.what()));
        }
    });

    object_->install_method_handler<location::dbus::Session::UpdateHeading>([this](const core::dbus::Message::Ptr& incoming)
    {
        VLOG(10) << __PRETTY_FUNCTION__;

        try
        {
            Update<Heading> update; incoming->reader() >> update;
            updates_.heading = update;
            connection_->send(core::dbus::Message::make_method_return(incoming));
        } catch(const std::runtime_error& e)
        {
            connection_->send(core::dbus::Message::make_error(
                                  incoming,
                                  location::dbus::Session::Errors::ErrorParsingUpdate::name(),
                                  e.what()));
        }
    });

    object_->install_method_handler<location::dbus::Session::UpdateVelocity>([this](const core::dbus::Message::Ptr& incoming)
    {
        VLOG(10) << __PRETTY_FUNCTION__;

        try
        {
            Update<Velocity> update; incoming->reader() >> update;
            updates_.velocity = update;
            connection_->send(core::dbus::Message::make_method_return(incoming));
        } catch(const std::runtime_error& e)
        {
            connection_->send(core::dbus::Message::make_error(
                                  incoming,
                                  location::dbus::Session::Errors::ErrorParsingUpdate::name(),
                                  e.what()));
        }
    });
}

location::dbus::stub::Session::~Session() noexcept
{
    VLOG(10) << __PRETTY_FUNCTION__;

    object_->uninstall_method_handler<location::dbus::Session::UpdatePosition>();
    object_->uninstall_method_handler<location::dbus::Session::UpdateHeading>();
    object_->uninstall_method_handler<location::dbus::Session::UpdateVelocity>();
}

void location::dbus::stub::Session::start_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    auto result = object_->transact_method<location::dbus::Session::StartPositionUpdates,void>();

    if (result.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
        throw std::runtime_error(ss.str());
    }
}

void location::dbus::stub::Session::stop_position_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    try
    {
        auto result = object_->invoke_method_synchronously<location::dbus::Session::StopPositionUpdates,void>();

        if (result.is_error())
        {
            std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
            throw std::runtime_error(ss.str());
        }
    } catch(const std::runtime_error& e)
    {
        VLOG(1) << e.what();
    }
}

void location::dbus::stub::Session::start_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    auto result = object_->transact_method<location::dbus::Session::StartVelocityUpdates,void>();

    if (result.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
        throw std::runtime_error(ss.str());
    }
}

void location::dbus::stub::Session::stop_velocity_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    try {
        auto result = object_->transact_method<location::dbus::Session::StopVelocityUpdates,void>();

        if (result.is_error())
        {
            std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
            throw std::runtime_error(ss.str());
        }
    } catch(const std::runtime_error& e)
    {
        VLOG(1) << e.what();
    }
}

void location::dbus::stub::Session::start_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    auto result = object_->transact_method<location::dbus::Session::StartHeadingUpdates,void>();

    if (result.is_error())
    {
        std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
        throw std::runtime_error(ss.str());
    }
}

void location::dbus::stub::Session::stop_heading_updates()
{
    VLOG(10) << __PRETTY_FUNCTION__;

    try {
        auto result = object_->transact_method<location::dbus::Session::StopHeadingUpdates,void>();

        if (result.is_error())
        {
            std::stringstream ss; ss << __PRETTY_FUNCTION__ << ": " << result.error().print();
            throw std::runtime_error(ss.str());
        }
    } catch(const std::runtime_error& e)
    {
        VLOG(1) << e.what();
    }
}

location::Service::Session::Updates& location::dbus::stub::Session::updates()
{
    return updates_;
}
