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
        d.visible_wireless_networks.getter = [this]()
        {
            auto devices = d.network_manager.get_devices();
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
                        wifi.frequency.set(ap.frequency->get());
                        wifi.snr = ap.strength->get() / 127.;

                        wifis.push_back(wifi);
                    }
                }
            }
            return std::vector<connectivity::WirelessNetwork>(wifis);
        };

        d.visible_radio_cells.getter = [this]()
        {
            std::vector<connectivity::RadioCell> cells;

            d.modem_manager.for_each_modem([&cells](const org::Ofono::Manager::Modem& modem)
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
                std::cout << "Successfully queried radio type." << std::endl;
                auto lac =
                        modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode
                        >();
                std::cout << "Successfully queried lac." << std::endl;
                auto cell_id =
                        modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::CellId
                        >();
                std::cout << "Successfully queried cell_id." << std::endl;
                auto strength =
                        modem.network_registration.get<
                            org::Ofono::Manager::Modem::NetworkRegistration::Strength
                        >();
                std::cout << "Successfully queried strength." << std::endl;
                std::stringstream ssmcc
                {
                    modem.network_registration.get<
                        org::Ofono::Manager::Modem::NetworkRegistration::MobileCountryCode
                    >()
                };
                int mcc; ssmcc >> mcc;
                std::cout << "Successfully queried mcc." << std::endl;
                std::stringstream ssmnc
                {
                    modem.network_registration.get<
                        org::Ofono::Manager::Modem::NetworkRegistration::MobileNetworkCode
                    >()
                };
                int mnc; ssmnc >> mnc;
                std::cout << "Successfully queried mnc." << std::endl;

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
                        connectivity::RadioCell::Gsm::RSS{-113},
                        connectivity::RadioCell::Gsm::ASU
                        {
                            static_cast<int>(0 + 31 * (strength / 127.))
                        },
                        connectivity::RadioCell::Gsm::TA{0}
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
                        connectivity::RadioCell::Lte::PID{0},
                        connectivity::RadioCell::Lte::RSS{-113},
                        connectivity::RadioCell::Lte::ASU
                        {
                            static_cast<int>(0 + 31 * (strength / 127.))
                        },
                        connectivity::RadioCell::Lte::TA{0}
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
                        connectivity::RadioCell::Umts::RSS{-113},
                        connectivity::RadioCell::Umts::ASU
                        {
                            static_cast<int>(0 + 31 * (strength / 127.))
                        }
                    };
                    cells.emplace_back(umts);
                    break;
                }
                case connectivity::RadioCell::Type::cdma:
                {
                    connectivity::RadioCell::Cdma cdma
                    {
                        connectivity::RadioCell::Cdma::MCC{mcc},
                        connectivity::RadioCell::Cdma::MNC{mnc},
                        connectivity::RadioCell::Cdma::LAC{lac},
                        connectivity::RadioCell::Cdma::ID{cell_id},
                        connectivity::RadioCell::Cdma::RSS{-113},
                        connectivity::RadioCell::Cdma::ASU
                        {
                            static_cast<int>(0 + 31 * (strength / 127.))
                        }
                    };
                    cells.emplace_back(cdma);
                    break;
                }
                default: break; // By default, we do not add a cell.
                }
            });

            std::cout << __PRETTY_FUNCTION__ << ": " << cells.size() << std::endl;

            return cells;
        };
    }

    const core::Property<std::vector<connectivity::WirelessNetwork>>& visible_wireless_networks()
    {
        return d.visible_wireless_networks;
    }

    const core::Property<std::vector<connectivity::RadioCell>>& visible_radio_cells()
    {
        return d.visible_radio_cells;
    }

    struct Private
    {
        Private()
            : network_manager(system_bus()),
              modem_manager(system_bus())
        {
        }

        org::freedesktop::NetworkManager network_manager;
        org::Ofono::Manager modem_manager;
        DispatchedProperty<std::vector<connectivity::WirelessNetwork>> visible_wireless_networks;
        DispatchedProperty<std::vector<connectivity::RadioCell>> visible_radio_cells;
    } d;
};
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new OfonoNmConnectivityManager{}};
    return instance;
}
