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

#ifndef LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_
#define LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_

#include <core/dbus/macros.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/signal.h>

#include <core/dbus/traits/service.h>

#include <location/dbus/codec.h>
#include <location/update.h>

#include <location/heading.h>
#include <location/position.h>
#include <location/velocity.h>

namespace location
{
namespace providers
{
namespace remote
{
struct Interface
{
    static const std::string& name()
    {
        static const std::string s{"com.ubuntu.remote.Service.Provider"};
        return s;
    }

    // Checks if a provider satisfies a set of accuracy criteria.
    DBUS_CPP_METHOD_DEF(MatchesCriteria, remote::Interface)
    // Checks if the provider has got a specific requirement.
    DBUS_CPP_METHOD_DEF(Requires, remote::Interface)
    // Checks if the provider supports a specific feature.
    DBUS_CPP_METHOD_DEF(Supports, remote::Interface)
    // Called by the engine whenever the wifi and cell ID reporting state changes.
    DBUS_CPP_METHOD_DEF(OnWifiAndCellIdReportingStateChanged, remote::Interface)
    // Called by the engine whenever the reference location changed.
    DBUS_CPP_METHOD_DEF(OnReferenceLocationChanged, remote::Interface)
    // Called by the engine whenever the reference heading changed.
    DBUS_CPP_METHOD_DEF(OnReferenceHeadingChanged, remote::Interface)
    // Called by the engine whenever the reference velocity changed.
    DBUS_CPP_METHOD_DEF(OnReferenceVelocityChanged, remote::Interface)

    DBUS_CPP_METHOD_DEF(StartPositionUpdates, remote::Interface)
    DBUS_CPP_METHOD_DEF(StopPositionUpdates, remote::Interface)
    DBUS_CPP_METHOD_DEF(StartHeadingUpdates, remote::Interface)
    DBUS_CPP_METHOD_DEF(StopHeadingUpdates, remote::Interface)
    DBUS_CPP_METHOD_DEF(StartVelocityUpdates, remote::Interface)
    DBUS_CPP_METHOD_DEF(StopVelocityUpdates, remote::Interface)

    struct Signals
    {
        DBUS_CPP_SIGNAL_DEF(PositionChanged, remote::Interface, location::Position)
        DBUS_CPP_SIGNAL_DEF(HeadingChanged, remote::Interface, location::Heading)
        DBUS_CPP_SIGNAL_DEF(VelocityChanged, remote::Interface, location::Velocity)
    };

    struct Properties
    {
        DBUS_CPP_READABLE_PROPERTY_DEF(HasPosition, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasVelocity, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(HasHeading, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresSatellites, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresCellNetwork, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresDataNetwork, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(RequiresMonetarySpending, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(ArePositionUpdatesRunning, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreHeadingUpdatesRunning, remote::Interface, bool)
        DBUS_CPP_READABLE_PROPERTY_DEF(AreVelocityUpdatesRunning, remote::Interface, bool)
    };

    struct Skeleton
    {
        // Creates a new skeleton instance and installs the interface
        // remote::Interface on it.
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
        // remote::Interface on it.
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
}
}
}

#endif // LOCATION_SERVICE_PROVIDERS_REMOTE_INTERFACE_H_
