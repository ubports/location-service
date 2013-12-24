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

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/types/object_path.h>
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
    system_bus()->stop();

    if (worker.joinable())
        worker.join();
}
}

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

        AccessPoint(const std::shared_ptr<dbus::Object>& object)
            : frequency(object->get_property<Frequency>()),
              hw_address(object->get_property<HwAddress>()),
              strength(object->get_property<Strength>())
        {
        }

        std::shared_ptr<dbus::Property<Frequency>> frequency;
        std::shared_ptr<dbus::Property<HwAddress>> hw_address;
        std::shared_ptr<dbus::Property<Strength>> strength;
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

        Device(const std::shared_ptr<dbus::Service>& service,
               const std::shared_ptr<dbus::Object>& object)
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
            typedef std::vector<dbus::types::ObjectPath> ResultType;
            auto result = object->invoke_method_synchronously<Wireless::GetAccessPoints, ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            std::vector<AccessPoint> aps;

            for (const auto& path : result.value())
                aps.push_back(AccessPoint(service->object_for_path(path)));

            return aps;
        }

        std::shared_ptr<dbus::Service> service;
        std::shared_ptr<dbus::Object> object;
        std::shared_ptr<dbus::Property<DeviceType>> device_type;
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

    NetworkManager()
        : service(dbus::Service::use_service<NetworkManager>(system_bus())),
          object(service->object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager")))
    {
    }

    std::vector<Device> get_devices()
    {
        auto result =
                object->invoke_method_synchronously<
                    NetworkManager::GetDevices,
                    std::vector<dbus::types::ObjectPath>>();

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

    std::shared_ptr<dbus::Service> service;
    std::shared_ptr<dbus::Object> object;
};
}
struct Ofono
{
    static const std::string& name()
    {
        static const std::string s{"org.ofono"};
        return s;
    }

    struct Manager
    {
        static const std::string& name()
        {
            static const std::string s{"org.ofono.Manager"};
            return s;
        }

        struct GetModems
        {
            static const std::string& name()
            {
                static const std::string s{"GetModems"};
                return s;
            }

            typedef Manager Interface;
            typedef std::vector<std::tuple<dbus::types::ObjectPath, std::map<std::string, std::string>>> ResultType;

            static std::chrono::milliseconds default_timeout()
            {
                return std::chrono::seconds{1};
            }
        };

        struct ModemAdded
        {
            static const std::string& name()
            {
                static const std::string s{"ModemAdded"};
                return s;
            }

            typedef Manager Interface;
            typedef std::tuple<dbus::types::ObjectPath, std::map<std::string, std::string>> ArgumentType;
        };

        struct ModemRemoved
        {
            static const std::string& name()
            {
                static const std::string s{"ModemRemoved"};
                return s;
            }

            typedef Manager Interface;
            typedef dbus::types::ObjectPath ArgumentType;
        };

        struct Modem
        {
            struct NetworkRegistration
            {
                static const std::string& name()
                {
                    static const std::string s{"org.ofono.NetworkRegistration"};
                    return s;
                }

                struct Mode
                {
                    static const char* unregistered() { return "unregistered"; }
                    static const char* registered() { return "registered"; }
                    static const char* searching() { return "searching"; }
                    static const char* denied() { return "denied"; }
                    static const char* unknown() { return "unknown"; }
                    static const char* roaming() { return "roaming"; }

                    static const std::string& name()
                    {
                        static const std::string s{"Mode"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct LocationAreaCode
                {
                    static const std::string& name()
                    {
                        static const std::string s{"LocationAreaCode"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
                    typedef std::uint16_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct CellId
                {
                    static const std::string& name()
                    {
                        static const std::string s{"CellId"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct MobileCountryCode
                {
                    static const std::string& name()
                    {
                        static const std::string s{"MobileCountryCode"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct MobileNetworkCode
                {
                    static const std::string& name()
                    {
                        static const std::string s{"MobileNetworkCode"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Technology
                {
                    static const char* gsm() { return "gsm"; }
                    static const char* edge() { return "edge"; }
                    static const char* umts() { return "umts"; }
                    static const char* hspa() { return "hspa"; }
                    static const char* lte() { return "lte"; }

                    static const std::string& name()
                    {
                        static const std::string s{"Technology"};
                        return s;
                    }

                    typedef NetworkRegistration Interface;
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

                    typedef NetworkRegistration Interface;
                    typedef std::int8_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                std::shared_ptr<dbus::Property<Mode>> mode;
                std::shared_ptr<dbus::Property<LocationAreaCode>> lac;
                std::shared_ptr<dbus::Property<CellId>> cell_id;
                std::shared_ptr<dbus::Property<MobileCountryCode>> mcc;
                std::shared_ptr<dbus::Property<MobileNetworkCode>> mnc;
                std::shared_ptr<dbus::Property<Technology>> technology;
                std::shared_ptr<dbus::Property<Strength>> strength;
            };

            Modem() = default;

            Modem(const std::shared_ptr<dbus::Service>& service,
                  const std::shared_ptr<dbus::Object>& object)
                : service(service),
                  object(object),
                  network_registration
                  {
                      object->get_property<NetworkRegistration::Mode>(),
                      object->get_property<NetworkRegistration::LocationAreaCode>(),
                      object->get_property<NetworkRegistration::CellId>(),
                      object->get_property<NetworkRegistration::MobileCountryCode>(),
                      object->get_property<NetworkRegistration::MobileNetworkCode>(),
                      object->get_property<NetworkRegistration::Technology>(),
                      object->get_property<NetworkRegistration::Strength>()
                  }
            {
            }

            std::shared_ptr<dbus::Service> service;
            std::shared_ptr<dbus::Object> object;
            NetworkRegistration network_registration;
        };

        Manager()
            : service(dbus::Service::use_service<Ofono>(system_bus())),
              object(service->object_for_path(dbus::types::ObjectPath("/"))),
              modem_added(object->get_signal<ModemAdded>()),
              modem_removed(object->get_signal<ModemRemoved>())
        {
            auto result = object->invoke_method_synchronously<GetModems, GetModems::ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            for (const auto& element : result.value())
            {
                modems[std::get<0>(element)] = Modem{
                        service,
                        service->object_for_path(std::get<0>(element))};
            }

            modem_added->connect([this](const ModemAdded::ArgumentType& arg)
            {
                std::lock_guard<std::mutex> lg(guard);
                modems[std::get<0>(arg)] = Modem{
                        service,
                        service->object_for_path(std::get<0>(arg))};
            });

            modem_removed->connect([this](const ModemRemoved::ArgumentType& arg)
            {
                std::lock_guard<std::mutex> lg(guard);
                modems.erase(arg);
            });
        }

        void for_each_modem(const std::function<void(const Modem&)>& functor) const
        {
            std::lock_guard<std::mutex> lg(guard);
            for (const auto& modem : modems)
            {
                functor(modem.second);
            }
        }

        std::shared_ptr<dbus::Service> service;
        std::shared_ptr<dbus::Object> object;
        std::shared_ptr<dbus::Signal<ModemAdded, ModemAdded::ArgumentType>> modem_added;
        std::shared_ptr<dbus::Signal<ModemRemoved, ModemRemoved::ArgumentType>> modem_removed;
        mutable std::mutex guard;
        std::map<dbus::types::ObjectPath, Modem> modems;
    };
};
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

                auto radio_type = type_lut.at(modem.network_registration.technology->get());
                auto lac = modem.network_registration.lac->get();
                auto cell_id = modem.network_registration.cell_id->get();
                auto strength = modem.network_registration.strength->get();
                std::stringstream ssmcc{modem.network_registration.mcc->get()};
                int mcc; ssmcc >> mcc;
                std::stringstream ssmnc{modem.network_registration.mnc->get()};
                int mnc; ssmnc >> mnc;

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

    struct
    {
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
