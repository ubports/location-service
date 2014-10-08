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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#ifndef MOCK_NETWORK_MANAGER_H_
#define MOCK_NETWORK_MANAGER_H_

#include <com/ubuntu/location/connectivity/nm.h>

#include <gmock/gmock.h>

namespace xdg = org::freedesktop;

namespace mock
{
struct NetworkManager
{
    struct AccessPoint
    {
        AccessPoint(const core::dbus::Object::Ptr& object)
            : object{object},
              properties
              {
                  object->get_property<xdg::NetworkManager::AccessPoint::Frequency>(),
                  object->get_property<xdg::NetworkManager::AccessPoint::LastSeen>(),
                  object->get_property<xdg::NetworkManager::AccessPoint::Mode>(),
                  object->get_property<xdg::NetworkManager::AccessPoint::HwAddress>(),
                  object->get_property<xdg::NetworkManager::AccessPoint::Ssid>(),
                  object->get_property<xdg::NetworkManager::AccessPoint::Strength>()

              },
              signals
              {
                  object->get_signal<xdg::NetworkManager::AccessPoint::PropertiesChanged>()
              }
        {
            properties.frequency->set(2412);
            properties.last_seen->set(0);
            properties.mode->set((std::uint32_t)xdg::NetworkManager::AccessPoint::Mode::infra);
        }

        core::dbus::Object::Ptr object;

        struct
        {
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::Frequency>> frequency;
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::LastSeen>> last_seen;
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::Mode>> mode;
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::HwAddress>> hw_address;
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::Ssid>> ssid;
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::AccessPoint::Strength>> strength;
        } properties;

        struct
        {
            std::shared_ptr<core::dbus::Signal<xdg::NetworkManager::AccessPoint::PropertiesChanged, xdg::NetworkManager::AccessPoint::PropertiesChanged::ArgumentType>> properties_changed;
        } signals;
    };

    struct Device
    {
        Device(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object)
            : bus{bus}, object{object},
              properties
              {
                  object->get_property<xdg::NetworkManager::Device::DeviceType>()
              },
              signals
              {
                  object->get_signal<xdg::NetworkManager::Device::Wireless::Signals::ScanDone>(),
                  object->get_signal<xdg::NetworkManager::Device::Wireless::Signals::AccessPointAdded>(),
                  object->get_signal<xdg::NetworkManager::Device::Wireless::Signals::AccessPointRemoved>()
              }
        {
            // To save us some typing.
            using namespace ::testing;

            properties.device_type->set(static_cast<std::uint32_t>(xdg::NetworkManager::Device::Type::unknown));

            ON_CALL(*this, get_access_points()).WillByDefault(Return(std::vector<core::dbus::types::ObjectPath>{}));

            object->install_method_handler<xdg::NetworkManager::Device::Wireless::GetAccessPoints>([this](const core::dbus::Message::Ptr& msg)
            {
                auto reply = core::dbus::Message::make_method_return(msg);
                reply->writer() << get_access_points();
                this->bus->send(reply);
            });

            object->install_method_handler<xdg::NetworkManager::Device::Wireless::RequestScan>([this](const core::dbus::Message::Ptr& msg)
            {
                auto reply = core::dbus::Message::make_method_return(msg);
                request_scan();
                this->bus->send(reply);
            });
        }

        MOCK_METHOD0(get_access_points, std::vector<core::dbus::types::ObjectPath>());
        MOCK_METHOD0(request_scan, void());

        core::dbus::Bus::Ptr bus;
        core::dbus::Object::Ptr object;

        struct
        {
            std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Device::DeviceType>> device_type;
        } properties;

        struct
        {
            core::dbus::Signal<xdg::NetworkManager::Device::Wireless::Signals::ScanDone, xdg::NetworkManager::Device::Wireless::Signals::ScanDone::ArgumentType>::Ptr scan_done;
            core::dbus::Signal<xdg::NetworkManager::Device::Wireless::Signals::AccessPointAdded, xdg::NetworkManager::Device::Wireless::Signals::AccessPointAdded::ArgumentType>::Ptr ap_added;
            core::dbus::Signal<xdg::NetworkManager::Device::Wireless::Signals::AccessPointRemoved, xdg::NetworkManager::Device::Wireless::Signals::AccessPointRemoved::ArgumentType>::Ptr ap_removed;
        } signals;
    };

    NetworkManager(const core::dbus::Bus::Ptr& bus, const core::dbus::Service::Ptr& service, const core::dbus::Object::Ptr& object)
        : bus{bus}, service{service}, object{object},
          properties
          {
              object->get_property<xdg::NetworkManager::Properties::Connectivity>(),
              object->get_property<xdg::NetworkManager::Properties::PrimaryConnection>(),
              object->get_property<xdg::NetworkManager::Properties::State>(),
              object->get_property<xdg::NetworkManager::Properties::WirelessEnabled>(),
              object->get_property<xdg::NetworkManager::Properties::WirelessHardwareEnabled>(),
              object->get_property<xdg::NetworkManager::Properties::WwanEnabled>(),
              object->get_property<xdg::NetworkManager::Properties::WwanHardwareEnabled>()
          },
          signals
          {
              object->get_signal<xdg::NetworkManager::Signals::DeviceAdded>(),
              object->get_signal<xdg::NetworkManager::Signals::DeviceRemoved>(),
              object->get_signal<xdg::NetworkManager::Signals::PropertiesChanged>(),
              object->get_signal<xdg::NetworkManager::Signals::StateChanged>()
          }
    {
        // To save us some typing.
        using namespace ::testing;

        ON_CALL(*this, get_devices()).WillByDefault(Return(std::vector<core::dbus::types::ObjectPath>{}));

        object->install_method_handler<xdg::NetworkManager::GetDevices>([this](const core::dbus::Message::Ptr& msg)
        {
            auto reply = core::dbus::Message::make_method_return(msg);
            reply->writer() << get_devices();
            this->bus->send(reply);
        });
    }

    MOCK_METHOD0(get_devices, std::vector<core::dbus::types::ObjectPath>());

    core::dbus::Bus::Ptr bus;
    core::dbus::Service::Ptr service;
    core::dbus::Object::Ptr object;

    struct
    {
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::Connectivity> > connectivity;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::PrimaryConnection> > primary_connection;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::State> > state;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::WirelessEnabled> > is_wifi_enabled;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::WirelessHardwareEnabled> > is_wifi_hardware_enabled;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::WwanEnabled> > is_wwan_enabled;
        std::shared_ptr<core::dbus::Property<xdg::NetworkManager::Properties::WwanHardwareEnabled> > is_wwan_hardware_enabled;
    } properties;

    struct
    {
        core::dbus::Signal<xdg::NetworkManager::Signals::DeviceAdded, xdg::NetworkManager::Signals::DeviceAdded::ArgumentType>::Ptr device_added;
        core::dbus::Signal<xdg::NetworkManager::Signals::DeviceRemoved, xdg::NetworkManager::Signals::DeviceRemoved::ArgumentType>::Ptr device_removed;
        core::dbus::Signal<xdg::NetworkManager::Signals::PropertiesChanged, xdg::NetworkManager::Signals::PropertiesChanged::ArgumentType>::Ptr properties_changed;
        core::dbus::Signal<xdg::NetworkManager::Signals::StateChanged, xdg::NetworkManager::Signals::StateChanged::ArgumentType>::Ptr state_changed;
    } signals;
};
}

#endif // MOCK_NETWORK_MANAGER_H_
