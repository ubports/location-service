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

struct CachedRadioCell : public connectivity::RadioCell
{
    typedef std::shared_ptr<CachedRadioCell> Ptr;

    static const std::map<std::string, connectivity::RadioCell::Type>& type_lut()
    {
        static const std::map<std::string, connectivity::RadioCell::Type> lut
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

        return lut;
    };

    CachedRadioCell(const org::Ofono::Manager::Modem& modem)
        : RadioCell(), radio_type(Type::gsm), modem(modem), detail{Gsm()}
    {
        auto technology =
                modem.network_registration.get<
                    org::Ofono::Manager::Modem::NetworkRegistration::Technology
                >();

        auto it = type_lut().find(technology);

        if (it == type_lut().end()) throw std::runtime_error
        {
            "Unknown technology for connected cell: " + technology
        };

        if (it->second == connectivity::RadioCell::Type::unknown) throw std::runtime_error
        {
            "Unknown technology for connected cell: " + technology
        };

        radio_type = it->second;

        auto lac =
                modem.network_registration.get<
                    org::Ofono::Manager::Modem::NetworkRegistration::LocationAreaCode
                >();

        int cell_id =
                modem.network_registration.get<
                    org::Ofono::Manager::Modem::NetworkRegistration::CellId
                >();

        auto strength =
                modem.network_registration.get<
                    org::Ofono::Manager::Modem::NetworkRegistration::Strength
                >();

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
            VLOG(1) << gsm;
            detail.gsm = gsm;
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
            VLOG(1) << lte;
            detail.lte = lte;
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
            VLOG(1) << umts;
            detail.umts = umts;
            break;
        }
        default:
            break;
        }

        modem.signals.property_changed->connect([this](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
        {
            VLOG(10) << "Property on modem " << CachedRadioCell::modem.object->path() << " changed: " << std::get<0>(tuple);
        });

        modem.network_registration.signals.property_changed->connect([this](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
        {
            VLOG(10) << "Property changed for network registration: " << std::get<0>(tuple);
        });
    }

    CachedRadioCell(const CachedRadioCell& rhs)
        : RadioCell(), radio_type(rhs.radio_type), modem(rhs.modem)
    {
        switch(radio_type)
        {
        case connectivity::RadioCell::Type::gsm: detail.gsm = rhs.detail.gsm; break;
        case connectivity::RadioCell::Type::umts: detail.umts = rhs.detail.umts; break;
        case connectivity::RadioCell::Type::lte: detail.lte = rhs.detail.lte; break;
        case connectivity::RadioCell::Type::unknown: break;
        }
    }

    CachedRadioCell& operator=(const CachedRadioCell& rhs)
    {
        radio_type = rhs.radio_type;
        modem = rhs.modem;

        switch(radio_type)
        {
        case connectivity::RadioCell::Type::gsm: detail.gsm = rhs.detail.gsm; break;
        case connectivity::RadioCell::Type::umts: detail.umts = rhs.detail.umts; break;
        case connectivity::RadioCell::Type::lte: detail.lte = rhs.detail.lte; break;
        case connectivity::RadioCell::Type::unknown: break;
        }

        return *this;
    }

    const core::Signal<>& changed() const override
    {
        return on_changed;
    }

    connectivity::RadioCell::Type type() const override
    {
        return radio_type;
    }

    const connectivity::RadioCell::Gsm& gsm() const override
    {
        if (radio_type != connectivity::RadioCell::Type::gsm)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.gsm;
    }

    const connectivity::RadioCell::Umts& umts() const override
    {
        if (radio_type != RadioCell::Type::umts)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.umts;
    }

    const connectivity::RadioCell::Lte& lte() const override
    {
        if (radio_type != RadioCell::Type::lte)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.lte;
    }

    /** @cond */
    core::Signal<> on_changed;
    Type radio_type;
    org::Ofono::Manager::Modem modem;

    struct None {};

    union Detail
    {
        Detail() : none(None{})
        {
        }

        Detail(const connectivity::RadioCell::Gsm& gsm) : gsm(gsm)
        {
        }

        Detail(const connectivity::RadioCell::Umts& umts) : umts(umts)
        {
        }

        Detail(const connectivity::RadioCell::Lte& lte) : lte(lte)
        {
        }

        None none;
        Gsm gsm;
        Umts umts;
        Lte lte;
    } detail;
    /** @endcond */
};

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
                VLOG(1) << "Properties on access point " << ssid_.get() << " changed: " << std::endl
                        << "  " << pair.first;

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
        d.signals.connected_cell_added;
    }

    const core::Signal<connectivity::RadioCell::Ptr>& connected_cell_removed() const override
    {
        d.signals.connected_cell_removed;
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
                network_manager.reset(new org::freedesktop::NetworkManager(system_bus()));

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
            }

            try
            {
                modem_manager.reset(new org::Ofono::Manager(system_bus()));

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

        DispatchedProperty<connectivity::State> state;
    } d;
};
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new OfonoNmConnectivityManager{}};
    return instance;
}
