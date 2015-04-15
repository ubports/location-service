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
 *              Manuel de la Pena <manuel.delapena@canonical.com>
 */

#pragma once

#include <core/dbus/object.h>
#include <core/dbus/signal.h>

#include "core/dbus/object.h"
#include "core/dbus/signal.h"

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

#include <thread>

#include "geoclue.h"

namespace fd = org::freedesktop;

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace geoclue
{
class Provider : public com::ubuntu::location::Provider
{

  typedef dbus::Signal<
      fd::Geoclue::Position::Signals::PositionChanged,
      fd::Geoclue::Position::Signals::PositionChanged::ArgumentType
  > PositionChanged;

  typedef dbus::Signal<
      fd::Geoclue::Velocity::Signals::VelocityChanged,
      fd::Geoclue::Velocity::Signals::VelocityChanged::ArgumentType
  > VelocityChanged;

  public:
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    struct Configuration
    {
        static std::string key_name() { return "name"; }
        static std::string key_path() { return "path"; }
        std::string name;
        std::string path;

        Provider::Features features = Provider::Features::none;
        Provider::Requirements requirements = Provider::Requirements::none;
    };

    Provider(const Configuration& config);
    ~Provider() noexcept;

    virtual bool matches_criteria(const Criteria&);

    virtual void start_position_updates() override;
    virtual void stop_position_updates() override;

    virtual void start_velocity_updates() override;
    virtual void stop_velocity_updates() override;

    virtual void start_heading_updates() override;
    virtual void stop_heading_updates() override;

  protected:
    // set to be protected instead of private so that they can be exposed for testing
    void start();
    void stop();

    void on_position_changed(const fd::Geoclue::Position::Signals::PositionChanged::ArgumentType& arg);
    void on_velocity_changed(const fd::Geoclue::Velocity::Signals::VelocityChanged::ArgumentType& arg);

  private:
    dbus::Bus::Ptr bus;
    dbus::Service::Ptr service;
    dbus::Object::Ptr object;
    PositionChanged::Ptr signal_position_changed;
    VelocityChanged::Ptr signal_velocity_changed;
    PositionChanged::SubscriptionToken position_updates_connection;
    VelocityChanged::SubscriptionToken velocity_updates_connection;

    std::thread worker;
};
}
}
}
}
}
