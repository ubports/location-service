/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Manuel de la Pena <manuel.delapena@canonical.com>
 */

#ifndef CORE_UBUNTU_ESPOO_PROVIDER_P_H_
#define CORE_UBUNTU_ESPOO_PROVIDER_P_H_

#include <core/dbus/macros.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/signal.h>

#include <core/dbus/traits/service.h>

#include <com/ubuntu/location/codec.h>
#include <com/ubuntu/location/update.h>

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/velocity.h>

namespace cul = com::ubuntu::location;

namespace com
{
namespace ubuntu
{
namespace remote
{
struct RemoteInterface
{
    static const std::string& name()
    {
        static const std::string s{"com.ubuntu.remote.Service.Provider"};
        return s;
    }

    DBUS_CPP_METHOD_DEF(StartPositionUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopPositionUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StartHeadingUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopHeadingUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StartVelocityUpdates, RemoteInterface)
    DBUS_CPP_METHOD_DEF(StopVelocityUpdates, RemoteInterface)

    struct Signals
    {
        DBUS_CPP_SIGNAL_DEF(PositionChanged, RemoteInterface, cul::Position)
        DBUS_CPP_SIGNAL_DEF(HeadingChanged, RemoteInterface, cul::Heading)
        DBUS_CPP_SIGNAL_DEF(VelocityChanged, RemoteInterface, cul::Velocity)
    };

    struct Properties
    {
        DBUS_CPP_READABLE_PROPERTY_DEF(HasPosition, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasVelocity, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasHeading, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresSatellites, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresCellNetwork, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresDataNetwork, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresMonetarySpending, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(ArePositionUpdatesRunning, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreHeadingUpdatesRunning, RemoteInterface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreVelocityUpdatesRunning, RemoteInterface, bool)
    };

    struct Skeleton
    {
        // Creates a new skeleton instance and installs the interface
        // com::ubuntu::remote::Interface on it.
        Skeleton(const core::dbus::Object::Ptr& object)
            : object{object},
              properties
              {
                  object->get_property<Properties::HasPosition>(),
                  object->get_property<Properties::HasVelocity>(),
                  object->get_property<Properties::HasHeading>(),
                  object->get_property<Properties::RequiresSatellites>(),
                  object->get_property<Properties::RequiresCellNetwork>(),
                  object->get_property<Properties::RequiresDataNetwork>(),
                  object->get_property<Properties::RequiresMonetarySpending>(),
                  object->get_property<Properties::ArePositionUpdatesRunning>(),
                  object->get_property<Properties::AreHeadingUpdatesRunning>(),
                  object->get_property<Properties::AreVelocityUpdatesRunning>()
              },
              signals
              {
                  object->get_signal<Signals::PositionChanged>(),
                  object->get_signal<Signals::HeadingChanged>(),
                  object->get_signal<Signals::VelocityChanged>()
              }
        {
        }

        // The object that the interface is installed on.
        core::dbus::Object::Ptr object;
        // All known properties.
        struct
        {
            std::shared_ptr<core::dbus::Property<Properties::HasPosition>> has_position;
            std::shared_ptr<core::dbus::Property<Properties::HasVelocity>> has_velocity;
            std::shared_ptr<core::dbus::Property<Properties::HasHeading>> has_heading;
            std::shared_ptr<core::dbus::Property<Properties::RequiresSatellites>> requires_satellites;
            std::shared_ptr<core::dbus::Property<Properties::RequiresCellNetwork>> requires_cell_network;
            std::shared_ptr<core::dbus::Property<Properties::RequiresDataNetwork>> requires_data_network;
            std::shared_ptr<core::dbus::Property<Properties::RequiresMonetarySpending>> requires_monetary_spending;
            std::shared_ptr<core::dbus::Property<Properties::ArePositionUpdatesRunning>> are_position_updates_running;
            std::shared_ptr<core::dbus::Property<Properties::AreHeadingUpdatesRunning>> are_heading_updates_running;
            std::shared_ptr<core::dbus::Property<Properties::AreVelocityUpdatesRunning>> are_velocity_updates_running;
        } properties;
        // All known signals.
        struct
        {
            std::shared_ptr<core::dbus::Signal<
                Signals::PositionChanged,
                Signals::PositionChanged::ArgumentType
            >> position_changed;

            std::shared_ptr<core::dbus::Signal<
                Signals::HeadingChanged,
                Signals::HeadingChanged::ArgumentType
            >> heading_changed;

            std::shared_ptr<core::dbus::Signal<
                Signals::VelocityChanged,
                Signals::VelocityChanged::ArgumentType
            >> velocity_changed;
        } signals;
    };

    struct Stub
    {
        // Creates a new skeleton instance and installs the interface
        // com::ubuntu::remote::Interface on it.
        Stub(const core::dbus::Object::Ptr& object)
            : object{object},
              properties
              {
                  object->get_property<Properties::HasPosition>(),
                  object->get_property<Properties::HasVelocity>(),
                  object->get_property<Properties::HasHeading>(),
                  object->get_property<Properties::RequiresSatellites>(),
                  object->get_property<Properties::RequiresCellNetwork>(),
                  object->get_property<Properties::RequiresDataNetwork>(),
                  object->get_property<Properties::RequiresMonetarySpending>(),
                  object->get_property<Properties::ArePositionUpdatesRunning>(),
                  object->get_property<Properties::AreHeadingUpdatesRunning>(),
                  object->get_property<Properties::AreVelocityUpdatesRunning>()
              },
              signals
              {
                  object->get_signal<Signals::PositionChanged>(),
                  object->get_signal<Signals::HeadingChanged>(),
                  object->get_signal<Signals::VelocityChanged>()
              }
        {
        }

        // The object that the interface is installed on.
        core::dbus::Object::Ptr object;
        // All known properties.
        struct
        {
            std::shared_ptr<core::dbus::Property<Properties::HasPosition>> has_position;
            std::shared_ptr<core::dbus::Property<Properties::HasVelocity>> has_velocity;
            std::shared_ptr<core::dbus::Property<Properties::HasHeading>> has_heading;
            std::shared_ptr<core::dbus::Property<Properties::RequiresSatellites>> requires_satellites;
            std::shared_ptr<core::dbus::Property<Properties::RequiresCellNetwork>> requires_cell_network;
            std::shared_ptr<core::dbus::Property<Properties::RequiresDataNetwork>> requires_data_network;
            std::shared_ptr<core::dbus::Property<Properties::RequiresMonetarySpending>> requires_monetary_spending;
            std::shared_ptr<core::dbus::Property<Properties::ArePositionUpdatesRunning>> are_position_updates_running;
            std::shared_ptr<core::dbus::Property<Properties::AreHeadingUpdatesRunning>> are_heading_updates_running;
            std::shared_ptr<core::dbus::Property<Properties::AreVelocityUpdatesRunning>> are_velocity_updates_running;
        } properties;
        // All known signals.
        struct
        {
            std::shared_ptr<core::dbus::Signal<
                Signals::PositionChanged,
                Signals::PositionChanged::ArgumentType
            >> position_changed;

            std::shared_ptr<core::dbus::Signal<
                Signals::HeadingChanged,
                Signals::HeadingChanged::ArgumentType
            >> heading_changed;

            std::shared_ptr<core::dbus::Signal<
                Signals::VelocityChanged,
                Signals::VelocityChanged::ArgumentType
            >> velocity_changed;
        } signals;
    };

};
} // remote
} // ubuntu
}  // core

#endif
