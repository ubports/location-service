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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_NM_H
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_NM_H

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

namespace org
{
namespace freedesktop
{
struct NetworkManager
{
    struct AccessPoint
    {
        static const std::string& name()
        {
            static const std::string s{"org.freedesktop.NetworkManager.AccessPoint"};
            return s;
        }

        struct Frequency
        {
            static const std::string& name()
            {
                static const std::string s{"Frequency"};
                return s;
            }

            typedef AccessPoint Interface;
            typedef std::uint32_t ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };

        struct HwAddress
        {
            static const std::string& name()
            {
                static const std::string s{"HwAddress"};
                return s;
            }

            typedef AccessPoint Interface;
            typedef std::string ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };

        struct Strength
        {
            static const std::string& name()
            {
                static const std::string s{"Strength"};
                return s;
            }

            typedef AccessPoint Interface;
            typedef std::int8_t ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };

        AccessPoint(const std::shared_ptr<core::dbus::Object>& object)
            : frequency(object->get_property<Frequency>()),
              hw_address(object->get_property<HwAddress>()),
              strength(object->get_property<Strength>())
        {
        }

        std::shared_ptr<core::dbus::Property<Frequency>> frequency;
        std::shared_ptr<core::dbus::Property<HwAddress>> hw_address;
        std::shared_ptr<core::dbus::Property<Strength>> strength;
    };

    struct Device
    {
        static const std::string& name()
        {
            static const std::string s{"org.freedesktop.NetworkManager.Device"};
            return s;
        }

        enum class Type
        {
            unknown = 0,
            ethernet = 1,
            wifi = 2,
            unused_1 = 3,
            unused_2 = 4,
            bluetooth = 5,
            olpc_mesh = 6,
            wimax = 7,
            modem = 8,
            infiniband = 9,
            bond = 10,
            vlan = 11,
            adsl = 12,
            bridge = 13
        };

        struct Wireless
        {
            static const std::string& name()
            {
                static const std::string s{"org.freedesktop.NetworkManager.Device.Wireless"};
                return s;
            }

            struct GetAccessPoints
            {
                static const std::string& name()
                {
                    static const std::string s{"GetAccessPoints"};
                    return s;
                }

                typedef Wireless Interface;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{1};
                }
            };
        };

        struct DeviceType
        {
            static const std::string& name()
            {
                static const std::string s{"DeviceType"};
                return s;
            }

            typedef Device Interface;
            typedef std::uint32_t ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };

        Device(const std::shared_ptr<core::dbus::Service>& service,
               const std::shared_ptr<core::dbus::Object>& object)
            : service(service),
              object(object),
              device_type(object->get_property<DeviceType>())
        {
        }

        Type type() const
        {
            return static_cast<Type>(device_type->get());
        }

        std::vector<AccessPoint> get_access_points() const
        {
            typedef std::vector<core::dbus::types::ObjectPath> ResultType;
            auto result = object->invoke_method_synchronously<Wireless::GetAccessPoints, ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            std::vector<AccessPoint> aps;

            for (const auto& path : result.value())
                aps.push_back(AccessPoint(service->object_for_path(path)));

            return aps;
        }

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object> object;
        std::shared_ptr<core::dbus::Property<DeviceType>> device_type;
    };

    static const std::string& name()
    {
        static const std::string s{"org.freedesktop.NetworkManager"};
        return s;
    }

    struct GetDevices
    {
        static const std::string& name()
        {
            static const std::string s{"GetDevices"};
            return s;
        }

        typedef NetworkManager Interface;

        static std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
        }
    };

    NetworkManager(const core::dbus::Bus::Ptr& bus)
        : service(core::dbus::Service::use_service<NetworkManager>(bus)),
          object(service->object_for_path(core::dbus::types::ObjectPath("/org/freedesktop/NetworkManager")))
    {
    }

    std::vector<Device> get_devices()
    {
        auto result =
                object->invoke_method_synchronously<
                    NetworkManager::GetDevices,
                    std::vector<core::dbus::types::ObjectPath>>();

        if (result.is_error())
            throw std::runtime_error(result.error().print());

        std::vector<Device> devices;
        for (const auto& path : result.value())
        {
            devices.emplace_back(
                        Device(
                            service,
                            service->object_for_path(path)));
        }

        return devices;
    }

    std::shared_ptr<core::dbus::Service> service;
    std::shared_ptr<core::dbus::Object> object;
};
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_NM_H
