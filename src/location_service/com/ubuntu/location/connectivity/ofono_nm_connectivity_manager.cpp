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

#include <com/ubuntu/location/connectivity/manager.h>

#include <com/ubuntu/location/logging.h>

#include "cached_radio_cell.h"
#include "cached_wireless_network.h"
#include "nm.h"
#include "ofono.h"

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

#include <core/dbus/asio/executor.h>

#include "../set_name_for_thread.h"

#include <chrono>

namespace connectivity = com::ubuntu::location::connectivity;
namespace dbus = core::dbus;

namespace
{
struct OfonoNmConnectivityManager : public connectivity::Manager
{
    OfonoNmConnectivityManager()
    {
    }

    const core::Property<connectivity::State>& state() const override
    {
        return d.state;
    }

    void request_scan_for_wireless_networks() override
    {
        std::lock_guard<std::mutex> lg(d.cached.guard);

        for (const auto& pair : d.cached.wireless_devices)
            pair.second.request_scan();
    }

    const core::Signal<connectivity::WirelessNetwork::Ptr>& wireless_network_added() const override
    {
        return d.signals.wireless_network_added;
    }

    const core::Signal<connectivity::WirelessNetwork::Ptr>& wireless_network_removed() const override
    {
        return d.signals.wireless_network_removed;
    }

    void enumerate_visible_wireless_networks(const std::function<void(const connectivity::WirelessNetwork::Ptr&)>& f) const override
    {
        std::lock_guard<std::mutex> lg(d.cached.guard);
        for (const auto& wifi : d.cached.wifis)
            f(wifi.second);
    }

    const core::Signal<connectivity::RadioCell::Ptr>& connected_cell_added() const override
    {
        return d.signals.connected_cell_added;
    }

    const core::Signal<connectivity::RadioCell::Ptr>& connected_cell_removed() const override
    {
        return d.signals.connected_cell_removed;
    }

    void enumerate_connected_radio_cells(const std::function<void(const connectivity::RadioCell::Ptr&)>& f) const override
    {
        std::lock_guard<std::mutex> lg(d.cached.guard);
        for (const auto& cell : d.cached.cells)
            f(cell.second);
    }

    struct Private
    {
        Private()
        {
            try
            {
                system_bus.reset(
                            new core::dbus::Bus(
                                core::dbus::WellKnownBus::system));

                executor = dbus::asio::make_executor(system_bus);

                system_bus->install_executor(executor);

                worker = std::move(std::thread
                {
                    [this]()
                    {
                        system_bus->run();
                    }
                });

            } catch(const std::exception& e)
            {
                LOG(ERROR) << e.what();
                return;
            }

            try
            {
                network_manager.reset(new org::freedesktop::NetworkManager(system_bus));

                // Seed the map of known wifi devices
                auto devices = network_manager->get_devices();

                for(const auto& device : devices)
                {
                    if (device.type() == org::freedesktop::NetworkManager::Device::Type::wifi)
                    {
                        cached.wireless_devices.insert(std::make_pair(device.object->path(), device));

                        device.signals.ap_added->connect([this, device](const core::dbus::types::ObjectPath& path)
                        {
                            org::freedesktop::NetworkManager::AccessPoint ap
                            {
                                network_manager->service->add_object_for_path(path)
                            };

                            auto wifi = std::make_shared<CachedWirelessNetwork>(device, ap);

                            // Scoping access to the cache to prevent deadlocks
                            // when clients of the API request an enumeration of
                            // visible wireless networks in response to the signal
                            // wireless_network_added being emitted.
                            {
                                std::lock_guard<std::mutex> lg(cached.guard);
                                cached.wifis[path] = std::make_shared<CachedWirelessNetwork>(device, ap);
                            }

                            signals.wireless_network_added(wifi);
                        });

                        device.signals.ap_removed->connect([this, device](const core::dbus::types::ObjectPath& path)
                        {
                            connectivity::WirelessNetwork::Ptr wifi;

                            // Scoping access to the cache to prevent deadlocks
                            // when clients of the API request an enumeration of
                            // visible wireless networks in response to the signal
                            // wireless_network_removed being emitted.
                            {
                                std::lock_guard<std::mutex> lg(cached.guard);
                                wifi = cached.wifis.at(path);
                                cached.wifis.erase(path);
                            }

                            signals.wireless_network_removed(wifi);
                        });

                        auto access_points = device.get_access_points();

                        for (const auto& ap : access_points)
                        {
                            auto path = ap.object->path();
                            cached.wifis.insert(std::make_pair(path, std::make_shared<CachedWirelessNetwork>(device, ap)));
                        }
                    }
                }

                // Query the initial connectivity state
                auto s = network_manager->properties.connectivity->get();

                switch (s)
                {
                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::unknown:
                    state.set(connectivity::State::unknown);
                    break;
                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::none:
                    state.set(connectivity::State::none);
                    break;
                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::portal:
                    state.set(connectivity::State::portal);
                    break;
                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::limited:
                    state.set(connectivity::State::limited);
                    break;
                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::full:
                    state.set(connectivity::State::full);
                    break;
                }

                // And we wire up to property changes here
                network_manager->signals.properties_changed->connect([this](const std::map<std::string, core::dbus::types::Variant>& dict)
                {
                    // We route by string
                    static const std::unordered_map<std::string, std::function<void(const core::dbus::types::Variant&)> > lut
                    {
                        {
                            org::freedesktop::NetworkManager::Properties::Connectivity::name(),
                            [this](const core::dbus::types::Variant& value)
                            {
                                auto s = value.as<org::freedesktop::NetworkManager::Properties::Connectivity::ValueType>();

                                switch (s)
                                {
                                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::unknown:
                                    state.set(connectivity::State::unknown);
                                    break;
                                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::none:
                                    state.set(connectivity::State::none);
                                    break;
                                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::portal:
                                    state.set(connectivity::State::portal);
                                    break;
                                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::limited:
                                    state.set(connectivity::State::limited);
                                    break;
                                case org::freedesktop::NetworkManager::Properties::Connectivity::Values::full:
                                    state.set(connectivity::State::full);
                                    break;
                                }
                            }
                        }
                    };

                    for (const auto& pair : dict)
                    {
                        VLOG(1) << "Property has changed: " << std::endl
                                << "  " << pair.first;

                        if (lut.count(pair.first) > 0)
                            lut.at(pair.first)(pair.second);
                    }
                });


            } catch(const std::runtime_error& e)
            {
                LOG(ERROR) << e.what();
            } catch(const std::exception& e)
            {
                LOG(ERROR) << e.what();
            }

            try
            {
                modem_manager.reset(new org::Ofono::Manager(system_bus));

                modem_manager->for_each_modem([this](const org::Ofono::Manager::Modem& modem)
                {
                    try
                    {
                        auto modem_path = modem.object->path();

                        modem.signals.property_changed->connect([this, modem_path](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
                        {
                            const auto& key = std::get<0>(tuple);

                            VLOG(10) << "Property changed for modem: " << std::get<0>(tuple);

                            if (org::Ofono::Manager::Modem::Properties::Interfaces::name() == key)
                            {
                                auto interfaces = std::get<1>(tuple).as<std::vector<std::string> >();

                                if (VLOG_IS_ON(10))
                                    for(const auto& interface : interfaces)
                                        VLOG(10) << interface;

                                auto it = std::find(
                                            interfaces.begin(),
                                            interfaces.end(),
                                            std::string{org::Ofono::Manager::Modem::Properties::Interfaces::network_registration});

                                CachedRadioCell::Ptr cell;

                                if (it == interfaces.end())
                                {
                                    {
                                        std::lock_guard<std::mutex> lg(cached.guard);
                                        cell = cached.cells.at(modem_path);
                                        cached.cells.erase(modem_path);
                                    }
                                    signals.connected_cell_removed(cell);
                                } else
                                {
                                    auto itt = cached.cells.find(modem_path);
                                    if (itt == cached.cells.end())
                                    {
                                        cell = std::make_shared<CachedRadioCell>(cached.modems.at(modem_path));
                                        {
                                            std::lock_guard<std::mutex> lg(cached.guard);
                                            cached.cells.insert(
                                                    std::make_pair(
                                                        modem_path,
                                                        cell));
                                        }
                                        signals.connected_cell_added(cell);
                                    }
                                }
                            }

                        });

                        cached.modems.insert(std::make_pair(modem_path, modem));
                        cached.cells.insert(std::make_pair(modem_path, std::make_shared<CachedRadioCell>(modem)));
                    } catch(const std::runtime_error& e)
                    {
                        LOG(WARNING) << "Exception while creating connected radio cell: " << e.what();
                    }
                });

                modem_manager->signals.modem_added->connect([this](const org::Ofono::Manager::ModemAdded::ArgumentType& arg)
                {
                    try
                    {
                        const auto& path = std::get<0>(arg);

                        auto modem = modem_manager->modem_for_path(path);

                        modem.signals.property_changed->connect([this, path](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
                        {
                            const auto& key = std::get<0>(tuple);

                            VLOG(10) << "Property changed for modem: " << std::get<0>(tuple);

                            if (org::Ofono::Manager::Modem::Properties::Interfaces::name() == key)
                            {
                                auto interfaces = std::get<1>(tuple).as<std::vector<std::string> >();

                                if (VLOG_IS_ON(10))
                                    for(const auto& interface : interfaces)
                                        VLOG(10) << interface;

                                auto it = std::find(
                                            interfaces.begin(),
                                            interfaces.end(),
                                            std::string{org::Ofono::Manager::Modem::Properties::Interfaces::network_registration});

                                CachedRadioCell::Ptr cell;

                                if (it == interfaces.end())
                                {
                                    {
                                        std::lock_guard<std::mutex> lg(cached.guard);
                                        cell = cached.cells.at(path);
                                        cached.cells.erase(path);
                                    }
                                    signals.connected_cell_removed(cell);
                                } else
                                {
                                    auto itt = cached.cells.find(path);
                                    if (itt == cached.cells.end())
                                    {
                                        cell = std::make_shared<CachedRadioCell>(cached.modems.at(path));
                                        {
                                            std::lock_guard<std::mutex> lg(cached.guard);
                                            cached.cells.insert(
                                                    std::make_pair(
                                                        path,
                                                        cell));
                                        }
                                        signals.connected_cell_added(cell);
                                    }
                                }
                            }

                        });

                        auto cell = std::make_shared<CachedRadioCell>(modem);
                        {
                            std::lock_guard<std::mutex> lg(cached.guard);
                            cached.modems.insert(std::make_pair(modem.object->path(), modem));
                            cached.cells.insert(std::make_pair(modem.object->path(), cell));
                        }
                        signals.connected_cell_added(cell);
                    } catch(const std::runtime_error& e)
                    {
                        LOG(WARNING) << "Exception while creating connected radio cell: " << e.what();
                    }
                });

                modem_manager->signals.modem_removed->connect([this](const core::dbus::types::ObjectPath& path)
                {
                    CachedRadioCell::Ptr cell;
                    {
                        std::lock_guard<std::mutex> lg(cached.guard);
                        cell = cached.cells.at(path);
                        cached.modems.erase(path);
                        cached.cells.erase(path);
                    }
                    signals.connected_cell_removed(cell);
                });

            } catch (const std::runtime_error& e)
            {
                LOG(ERROR) << "Error while setting up access to telephony stack [" << e.what() << "]";
            }
        }

        ~Private()
        {
            if (system_bus)
                system_bus->stop();

            if (worker.joinable())
                worker.join();
        }

        core::dbus::Bus::Ptr system_bus;
        core::dbus::Executor::Ptr executor;

        std::thread worker;

        org::freedesktop::NetworkManager::Ptr network_manager;
        org::Ofono::Manager::Ptr modem_manager;

        struct
        {
            mutable std::mutex guard;
            std::map<core::dbus::types::ObjectPath, CachedRadioCell::Ptr> cells;
            std::map<core::dbus::types::ObjectPath, org::Ofono::Manager::Modem> modems;
            std::map<core::dbus::types::ObjectPath, CachedWirelessNetwork::Ptr> wifis;
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::Device> wireless_devices;
        } cached;

        struct
        {
            core::Signal<connectivity::RadioCell::Ptr> connected_cell_added;
            core::Signal<connectivity::RadioCell::Ptr> connected_cell_removed;
            core::Signal<connectivity::WirelessNetwork::Ptr> wireless_network_added;
            core::Signal<connectivity::WirelessNetwork::Ptr> wireless_network_removed;
        } signals;

        core::Property<connectivity::State> state;
    } d;
};
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new OfonoNmConnectivityManager{}};
    return instance;
}
