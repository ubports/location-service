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
struct OfonoNmConnectivityManager : public connectivity::Manager
{
    OfonoNmConnectivityManager()
    {
        if (d.network_manager)
        {
            d.visible_wireless_networks.getter = [this]()
            {
                auto devices = d.network_manager->get_devices();
                std::vector<connectivity::WirelessNetwork> wifis;
                for(const auto& device : devices)
                {
                    if (device.type() == org::freedesktop::NetworkManager::Device::Type::wifi)
                    {
                        auto aps = device.get_access_points();
                        for(auto const& ap : aps)
                        {
                            connectivity::WirelessNetwork wifi;
                            wifi.bssid = ap.hw_address->get();
                            auto ssid = ap.ssid->get();
                            wifi.ssid = std::string(ssid.begin(), ssid.end());

                            auto mode = ap.mode->get();

                            switch (mode)
                            {
                            case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::unknown:
                                wifi.mode = connectivity::WirelessNetwork::Mode::unknown;
                                break;
                            case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::adhoc:
                                wifi.mode = connectivity::WirelessNetwork::Mode::adhoc;
                                break;
                            case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::infra:
                                wifi.mode = connectivity::WirelessNetwork::Mode::infrastructure;
                                break;
                            }

                            wifi.frequency.set(ap.frequency->get());
                            wifi.snr = ap.strength->get() / 127.;

                            wifis.push_back(wifi);
                        }
                    }
                }
                return std::vector<connectivity::WirelessNetwork>(wifis);
            };
        }

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
                            connectivity::RadioCell::Lte::LAC{lac},
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
                            connectivity::RadioCell::Umts::PSC{},
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

    void request_scan_for_wireless_networks()
    {
        auto devices = d.network_manager->get_devices();

        for(const auto& device : devices)
        {
            if (device.type() == org::freedesktop::NetworkManager::Device::Type::wifi)
            {
                device.request_scan();
            }
        }
    }

    const core::Property<std::vector<connectivity::WirelessNetwork> >& visible_wireless_networks()
    {
        return d.visible_wireless_networks;
    }

    const core::Property<std::vector<connectivity::RadioCell> >& connected_radio_cells()
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
        DispatchedProperty<std::vector<connectivity::WirelessNetwork> > visible_wireless_networks;
        DispatchedProperty<std::vector<connectivity::RadioCell> > visible_radio_cells;
    } d;
};
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new OfonoNmConnectivityManager{}};
    return instance;
}
