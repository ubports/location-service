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
template<typename T>
struct DispatchedProperty : public core::Property<T>
{
    typedef std::function<T()> Getter;

    DispatchedProperty(const Getter& getter = Getter()) : getter(getter)
    {
    }

    const T& get() const
    {
        if (getter)
        {
            return core::Property<T>::mutable_get() = getter();
        }

        return core::Property<T>::get();
    }

    Getter getter;
};

struct State
{
    ~State();

    std::thread worker;
} state;

const std::shared_ptr<dbus::Bus>& system_bus()
{
    static std::once_flag once;
    static const std::shared_ptr<dbus::Bus> instance
    {
        new dbus::Bus(dbus::WellKnownBus::system)
    };

    std::call_once(once,[]
    {
        auto executor = dbus::asio::make_executor(instance);
        instance->install_executor(executor);
        state.worker = std::move(std::thread([]()
        {
            instance->run();
        }));
        com::ubuntu::location::set_name_for_thread(
                    state.worker,
                    "OfonoNmConnectivityManagerWorkerThread");
    });

    return instance;
}

State::~State()
{
    if (worker.joinable())
    {
        system_bus()->stop();
        worker.join();
    }
}
}

namespace
{

struct CachedWirelessNetwork : public connectivity::WirelessNetwork
{
    typedef std::shared_ptr<CachedWirelessNetwork> Ptr;

    const core::Property<std::chrono::system_clock::time_point>& timestamp() const override
    {
        return timestamp_;
    }

    const core::Property<std::string>& bssid() const override
    {
        return bssid_;
    }

    const core::Property<std::string>& ssid() const override
    {
        return ssid_;
    }

    const core::Property<Mode>& mode() const override
    {
        return mode_;
    }

    const core::Property<Frequency>& frequency() const override
    {
        return frequency_;
    }

    const core::Property<SignalStrength>& signal_strength() const override
    {
        return signal_strength_;
    }

    CachedWirelessNetwork(
            const org::freedesktop::NetworkManager::Device& device,
            const org::freedesktop::NetworkManager::AccessPoint& ap)
        : device_(device),
          access_point_(ap)
    {
        timestamp_ = std::chrono::system_clock::now();

        bssid_ = access_point_.hw_address->get();
        auto ssid = access_point_.ssid->get();
        ssid_ = std::string(ssid.begin(), ssid.end());

        auto mode = access_point_.mode->get();

        switch (mode)
        {
        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::unknown:
            mode_ = connectivity::WirelessNetwork::Mode::unknown;
            break;
        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::adhoc:
            mode_ = connectivity::WirelessNetwork::Mode::adhoc;
            break;
        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::infra:
            mode_ = connectivity::WirelessNetwork::Mode::infrastructure;
            break;
        }

        frequency_ = connectivity::WirelessNetwork::Frequency
        {
            access_point_.frequency->get()
        };

        signal_strength_ = connectivity::WirelessNetwork::SignalStrength
        {
            int(access_point_.strength->get())
        };

        // Wire up all the connections
        access_point_.properties_changed->connect([this](const std::map<std::string, core::dbus::types::Variant>& dict)
        {
            std::cout << "Properties on access point " << ssid_.get() << " changed: " << std::endl;
            // We route by string
            static const std::unordered_map<std::string, std::function<void(CachedWirelessNetwork&, const core::dbus::types::Variant&)> > lut
            {
                {
                    org::freedesktop::NetworkManager::AccessPoint::HwAddress::name(),
                    [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                    {
                        thiz.bssid_ = value.as<org::freedesktop::NetworkManager::AccessPoint::HwAddress::ValueType>();
                    }
                },
                {
                    org::freedesktop::NetworkManager::AccessPoint::Ssid::name(),
                    [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                    {
                        auto ssid = value.as<org::freedesktop::NetworkManager::AccessPoint::Ssid::ValueType>();
                        thiz.ssid_ = std::string(ssid.begin(), ssid.end());
                    }
                },
                {
                    org::freedesktop::NetworkManager::AccessPoint::Strength::name(),
                    [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                    {
                        thiz.signal_strength_ = connectivity::WirelessNetwork::SignalStrength
                        {
                            value.as<org::freedesktop::NetworkManager::AccessPoint::Strength::ValueType>()
                        };
                    }
                },
                {
                    org::freedesktop::NetworkManager::AccessPoint::Frequency::name(),
                    [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                    {
                        thiz.frequency_ = connectivity::WirelessNetwork::Frequency
                        {
                            value.as<org::freedesktop::NetworkManager::AccessPoint::Frequency::ValueType>()
                        };
                    }
                },
                {
                    org::freedesktop::NetworkManager::AccessPoint::Mode::name(),
                    [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                    {
                        switch (value.as<org::freedesktop::NetworkManager::AccessPoint::Mode::ValueType>())
                        {
                        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::unknown:
                            thiz.mode_ = connectivity::WirelessNetwork::Mode::unknown;
                            break;
                        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::adhoc:
                            thiz.mode_ = connectivity::WirelessNetwork::Mode::adhoc;
                            break;
                        case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::infra:
                            thiz.mode_ = connectivity::WirelessNetwork::Mode::infrastructure;
                            break;
                        }
                    }
                }
            };

            for (const auto& pair : dict)
            {
                std::cout << "  " << pair.first << std::endl;

                if (lut.count(pair.first) > 0)
                    lut.at(pair.first)(*this, pair.second);
            }
        });
    }

    org::freedesktop::NetworkManager::Device device_;
    org::freedesktop::NetworkManager::AccessPoint access_point_;

    core::Property<std::chrono::system_clock::time_point> timestamp_;
    core::Property<std::string> bssid_;
    core::Property<std::string> ssid_;
    core::Property<WirelessNetwork::Mode> mode_;
    core::Property<WirelessNetwork::Frequency> frequency_;
    core::Property<WirelessNetwork::SignalStrength> signal_strength_;
};

struct OfonoNmConnectivityManager : public connectivity::Manager
{
    OfonoNmConnectivityManager()
    {
        if (d.modem_manager)
        {
            d.visible_radio_cells.getter = [this]()
            {
                std::vector<connectivity::RadioCell> cells;

                d.modem_manager->for_each_modem([&cells](const org::Ofono::Manager::Modem& modem)
                {
                    static const std::map<std::string, connectivity::RadioCell::Type> type_lut =
                    {
                        {
                            org::Ofono::Manager::Modem::NetworkRegistration::Technology::gsm(),
                            connectivity::RadioCell::Type::gsm
                        },
                        {
                            org::Ofono::Manager::Modem::NetworkRegistration::Technology::lte(),
                            connectivity::RadioCell::Type::lte
                        },
                        {
                            org::Ofono::Manager::Modem::NetworkRegistration::Technology::umts(),
                            connectivity::RadioCell::Type::umts
                        },
                        {
                            org::Ofono::Manager::Modem::NetworkRegistration::Technology::edge(),
                            connectivity::RadioCell::Type::unknown
                        },
                        {
                            org::Ofono::Manager::Modem::NetworkRegistration::Technology::hspa(),
                            connectivity::RadioCell::Type::unknown
                        },
                        {std::string(), connectivity::RadioCell::Type::unknown}
                    };

                    auto radio_type = type_lut.at(
                                modem.network_registration.get<
                                org::Ofono::Manager::Modem::NetworkRegistration::Technology
                                >());
                    auto lac =
                            modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode
                            >();

                    int cell_id =
                            modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::CellId
                            >(0);

                    auto strength =
                            modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::Strength
                            >(0);

                    std::stringstream ssmcc
                    {
                        modem.network_registration.get<
                                org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode
                                >()
                    };
                    int mcc{0}; ssmcc >> mcc;
                    std::stringstream ssmnc
                    {
                        modem.network_registration.get<
                                org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode
                                >()
                    };
                    int mnc{0}; ssmnc >> mnc;

                    switch(radio_type)
                    {
                    case connectivity::RadioCell::Type::gsm:
                    {
                        connectivity::RadioCell::Gsm gsm
                        {
                            connectivity::RadioCell::Gsm::MCC{mcc},
                            connectivity::RadioCell::Gsm::MNC{mnc},
                            connectivity::RadioCell::Gsm::LAC{lac},
                            connectivity::RadioCell::Gsm::ID{cell_id},
                            connectivity::RadioCell::Gsm::SignalStrength::from_percent(strength/100.f)
                        };
                        cells.emplace_back(gsm);
                        break;
                    }
                    case connectivity::RadioCell::Type::lte:
                    {
                        connectivity::RadioCell::Lte lte
                        {
                            connectivity::RadioCell::Lte::MCC{mcc},
                            connectivity::RadioCell::Lte::MNC{mnc},
                            connectivity::RadioCell::Lte::TAC{lac},
                            connectivity::RadioCell::Lte::ID{cell_id},
                            connectivity::RadioCell::Lte::PID{},
                            connectivity::RadioCell::Lte::SignalStrength::from_percent(strength/100.f)
                        };
                        cells.emplace_back(lte);
                        break;
                    }
                    case connectivity::RadioCell::Type::umts:
                    {
                        connectivity::RadioCell::Umts umts
                        {
                            connectivity::RadioCell::Umts::MCC{mcc},
                            connectivity::RadioCell::Umts::MNC{mnc},
                            connectivity::RadioCell::Umts::LAC{lac},
                            connectivity::RadioCell::Umts::ID{cell_id},
                            connectivity::RadioCell::Umts::SignalStrength::from_percent(strength/100.f)
                        };
                        cells.emplace_back(umts);
                        break;
                    }
                    default: break; // By default, we do not add a cell.
                    }
                });

                return cells;
            };
        }
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

    const core::Property<std::vector<connectivity::WirelessNetwork::Ptr> >& visible_wireless_networks() override
    {
        return d.visible_wireless_networks;
    }

    const core::Property<std::vector<connectivity::RadioCell> >& connected_radio_cells() override
    {
        return d.visible_radio_cells;
    }

    struct Private
    {
        Private()
        {
            try
            {
                network_manager.reset(new org::freedesktop::NetworkManager(system_bus()));

                // Seed the map of known wifi devices
                auto devices = network_manager->get_devices();

                for(const auto& device : devices)
                {
                    if (device.type() == org::freedesktop::NetworkManager::Device::Type::wifi)
                    {
                        std::cout << device.object->path() << std::endl;
                        cached.wireless_devices.insert(std::make_pair(device.object->path(), device));

                        device.signals.ap_added->connect([this, device](const core::dbus::types::ObjectPath& path)
                        {
                            org::freedesktop::NetworkManager::AccessPoint ap
                            {
                                network_manager->service->add_object_for_path(path)
                            };

                            std::lock_guard<std::mutex> lg(cached.guard);
                            cached.wifis[path] = std::make_shared<CachedWirelessNetwork>(device, ap);

                            std::vector<connectivity::WirelessNetwork::Ptr> wifis;
                            for (const auto& wifi : cached.wifis)
                                wifis.push_back(wifi.second);

                            visible_wireless_networks.set(wifis);
                        });

                        device.signals.ap_removed->connect([this, device](const core::dbus::types::ObjectPath& path)
                        {
                            std::lock_guard<std::mutex> lg(cached.guard);
                            cached.wifis.erase(path);

                            std::vector<connectivity::WirelessNetwork::Ptr> wifis;
                            for (const auto& wifi : cached.wifis)
                                wifis.push_back(wifi.second);

                            visible_wireless_networks.set(wifis);
                        });

                        auto access_points = device.get_access_points();

                        for (const auto& ap : access_points)
                        {
                            auto path = ap.object->path();
                            cached.wifis.insert(std::make_pair(path, std::make_shared<CachedWirelessNetwork>(device, ap)));
                        }
                    }
                }

                std::vector<connectivity::WirelessNetwork::Ptr> wifis;
                for (const auto& wifi : cached.wifis)
                    wifis.push_back(wifi.second);

                visible_wireless_networks.set(wifis);

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
                                std::cout << "Connectivity status changed." << std::endl;

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
                        std::cout << pair.first << " changed" << std::endl;

                        if (lut.count(pair.first) > 0)
                            lut.at(pair.first)(pair.second);
                    }
                });


            } catch(const std::runtime_error& e)
            {
                std::cerr << e.what() << std::endl;
            }

            try
            {
                modem_manager.reset(new org::Ofono::Manager(system_bus()));
            } catch (const std::runtime_error& e)
            {
                std::cerr << e.what() << std::endl;
            }
        }

        org::freedesktop::NetworkManager::Ptr network_manager;
        org::Ofono::Manager::Ptr modem_manager;

        struct
        {
            std::mutex guard;
            std::map<core::dbus::types::ObjectPath, CachedWirelessNetwork::Ptr> wifis;
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::Device> wireless_devices;
        } cached;

        DispatchedProperty<connectivity::State> state;
        DispatchedProperty<std::vector<connectivity::WirelessNetwork::Ptr> > visible_wireless_networks;
        DispatchedProperty<std::vector<connectivity::RadioCell> > visible_radio_cells;
    } d;
};
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new OfonoNmConnectivityManager{}};
    return instance;
}
