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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_OFONO_H
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_OFONO_H

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/variant.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <com/ubuntu/location/logging.h>

namespace org
{
struct Ofono
{
static const std::string& name()
{
    static const std::string s{"org.ofono"};
    return s;
}
struct Manager
{
    typedef std::shared_ptr<Manager> Ptr;

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
        typedef std::vector<
            core::dbus::types::Struct<
                core::dbus::types::ObjectPath
            >
        > ResultType;

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
        typedef std::tuple<core::dbus::types::ObjectPath, std::map<std::string, std::string>> ArgumentType;
    };

    struct ModemRemoved
    {
        static const std::string& name()
        {
            static const std::string s{"ModemRemoved"};
            return s;
        }

        typedef Manager Interface;
        typedef core::dbus::types::ObjectPath ArgumentType;
    };

    struct Modem
    {
        static const std::string& name()
        {
            static const std::string s{"org.ofono.Modem"};
            return s;
        }

        struct GetProperties
        {
            static const std::string& name()
            {
                static const std::string s{"GetProperties"};
                return s;
            }

            typedef Modem Interface;
            typedef std::map<std::string, core::dbus::types::Variant> ValueType;

            static std::chrono::milliseconds default_timeout()
            {
                return std::chrono::seconds{1};
            }
        };

        struct PropertyChanged
        {
            inline static std::string name()
            {
                return "PropertyChanged";
            }

            typedef Modem Interface;

            typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
        };

        struct Properties
        {
            struct Powered
            {
                static const std::string& name()
                {
                    static const std::string s{"Powered"};
                    return s;
                }

                typedef Modem Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = true;
            };

            struct Online
            {
                static const std::string& name()
                {
                    static const std::string s{"Online"};
                    return s;
                }

                typedef Modem Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = true;
            };

            struct Lockdown
            {
                static const std::string& name()
                {
                    static const std::string s{"Lockdown"};
                    return s;
                }

                typedef Modem Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = true;
            };

            struct Name
            {
                static const std::string& name()
                {
                    static const std::string s{"Name"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Manufacturer
            {
                static const std::string& name()
                {
                    static const std::string s{"Manufacturer"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Model
            {
                static const std::string& name()
                {
                    static const std::string s{"Model"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Revision
            {
                static const std::string& name()
                {
                    static const std::string s{"Revision"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Serial
            {
                static const std::string& name()
                {
                    static const std::string s{"Serial"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Features
            {
                static constexpr const char* net{"net"};
                static constexpr const char* rat{"rat"};
                static constexpr const char* cbs{"cbs"};
                static constexpr const char* sms{"sms"};
                static constexpr const char* sim{"sim"};
                static constexpr const char* stk{"stk"};
                static constexpr const char* ussd{"ussd"};
                static constexpr const char* gprs{"gprs"};
                static constexpr const char* tty{"tty"};
                static constexpr const char* gps{"gps"};

                static const std::string& name()
                {
                    static const std::string s{"Features"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::vector<std::string> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Interfaces
            {
                static constexpr const char* assisted_satellite_navigation
                {
                    "org.ofono.AssistedSatelliteNavigation"
                };
                static constexpr const char* audio_settings
                {
                    "org.ofono.AudioSettings"
                };
                static constexpr const char* call_barring
                {
                    "org.ofono.CallBarring"
                };
                static constexpr const char* call_forwarding
                {
                    "org.ofono.CallForwarding"
                };
                static constexpr const char* call_meter
                {
                    "org.ofono.CallMeter"
                };
                static constexpr const char* call_settings
                {
                    "org.ofono.CallSettings"
                };
                static constexpr const char* call_volume
                {
                    "org.ofono.CallVolume"
                };
                static constexpr const char* cell_broadcast
                {
                    "org.ofono.CellBroadcast"
                };
                static constexpr const char* hands_free
                {
                    "org.ofono.Handsfree"
                };
                static constexpr const char* location_reporting
                {
                    "org.ofono.LocationReporting"
                };
                static constexpr const char* message_manager
                {
                    "org.ofono.MessageManager"
                };
                static constexpr const char* message_waiting
                {
                    "org.ofono.MessageWaiting"
                };
                static constexpr const char* network_registration
                {
                    "org.ofono.NetworkRegistration"
                };
                static constexpr const char* phonebook
                {
                    "org.ofono.Phonebook"
                };
                static constexpr const char* push_notification
                {
                    "org.ofono.PushNotification"
                };
                static constexpr const char* radio_settings
                {
                    "org.ofono.RadioSettings"
                };
                static constexpr const char* sim_manager
                {
                    "org.ofono.SimManager"
                };
                static constexpr const char* smart_messaging
                {
                    "org.ofono.SmartMessaging"
                };
                static constexpr const char* sim_toolkit
                {
                    "org.ofono.SimToolkit"
                };
                static constexpr const char* supplementary_services
                {
                    "org.ofono.SupplementaryServices"
                };
                static constexpr const char* text_telephony
                {
                    "org.ofono.TextTelephony"
                };
                static constexpr const char* voice_call_manager
                {
                    "org.ofono.VoiceCallManager"
                };

                static const std::string& name()
                {
                    static const std::string s{"Interfaces"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::vector<std::string> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Type
            {
                static constexpr const char* test{"test"};
                static constexpr const char* hfp{"hfp"};
                static constexpr const char* sap{"sap"};
                static constexpr const char* hardware{"hardware"};

                static const std::string& name()
                {
                    static const std::string s{"Type"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };

        struct NetworkRegistration
        {
            static const std::string& name()
            {
                static const std::string s{"org.ofono.NetworkRegistration"};
                return s;
            }            

            struct GetProperties
            {
                static const std::string& name()
                {
                    static const std::string s{"GetProperties"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::map<std::string, core::dbus::types::Variant> ValueType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{1};
                }
            };

            struct PropertyChanged
            {
                inline static std::string name()
                {
                    return "PropertyChanged";
                }

                typedef NetworkRegistration Interface;

                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
            };

            struct Status
            {
                enum class Value
                {
                    unregistered,
                    registered,
                    searching,
                    denied,
                    unknown,
                    roaming
                };

                static constexpr const char* unregistered
                {
                    "unregistered"
                };
                static constexpr const char* registered
                {
                    "registered"
                };
                static constexpr const char* searching
                {
                    "searching"
                };
                static constexpr const char* denied
                {
                    "denied"
                };
                static constexpr const char* unknown
                {
                    "unknown"
                };
                static constexpr const char* roaming
                {
                    "roaming"
                };

                static const char* value_to_string(Value value)
                {
                    switch (value)
                    {
                    case Value::unregistered: return unregistered;
                    case Value::registered: return registered;
                    case Value::searching: return searching;
                    case Value::denied: return denied;
                    case Value::unknown: return unknown;
                    case Value::roaming: return roaming;
                    }

                    return nullptr;
                }

                static const std::string& name()
                {
                    static const std::string s{"Status"};
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

            NetworkRegistration(const std::shared_ptr<core::dbus::Object>& object)
                : object(object),
                  signals
                  {
                      object->get_signal<PropertyChanged>()
                  }
            {
            }

            // Calling this function might fail. However, we do not throw and only log
            // the issue for later forensics.
            bool refresh_properties() const
            {
                auto result = object->invoke_method_synchronously<GetProperties, GetProperties::ValueType>();

                if (result.is_error())
                {
                    LOG(WARNING) << "Could not refresh properties for org::Ofono::Modem::NetworkRegistration: " << result.error().print();
                    return false;
                }

                properties = result.value();
                return true;
            }

            template<typename Property>
            typename Property::ValueType get(bool refresh = true) const
            {
                if (refresh)
                {
                    if (not refresh_properties())
                        return typename Property::ValueType{};
                }

                auto it = properties.find(Property::name());

                if (it == properties.end())
                {
                    VLOG(1) << "Could not find property for name " << Property::name();
                    return typename Property::ValueType{};
                }

                return it->second.template as<typename Property::ValueType>();
            }

            std::shared_ptr<core::dbus::Object> object;
            mutable GetProperties::ValueType properties;

            struct
            {
                core::dbus::Signal<PropertyChanged, PropertyChanged::ArgumentType>::Ptr property_changed;
            } signals;
        };

        Modem(const std::shared_ptr<core::dbus::Service>& service,
              const std::shared_ptr<core::dbus::Object>& object)
            : service(service),
              object(object),
              signals
              {
                  object->get_signal<PropertyChanged>()
              },
              network_registration{object}
        {
        }

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object> object;

        struct
        {
            core::dbus::Signal<PropertyChanged, PropertyChanged::ArgumentType>::Ptr property_changed;
        } signals;

        NetworkRegistration network_registration;
    };

    Manager(const core::dbus::Bus::Ptr& bus)
        : service(core::dbus::Service::use_service<org::Ofono>(bus)),
          object(service->object_for_path(core::dbus::types::ObjectPath("/"))),
          signals
          {
              object->get_signal<ModemAdded>(),
              object->get_signal<ModemRemoved>()
          }
    {
    }

    Modem modem_for_path(const core::dbus::types::ObjectPath& path) const
    {
        return Modem
        {
            service,
            service->object_for_path(path)
        };
    }

    void for_each_modem(const std::function<void(const core::dbus::types::ObjectPath&)>& functor) const
    {
        auto result = object->invoke_method_synchronously<GetModems, GetModems::ResultType>();

        if (result.is_error())
            throw std::runtime_error(result.error().print());

        for (const auto& element : result.value())
        {
            functor(element.value);
        }
    }

    std::shared_ptr<core::dbus::Service> service;
    std::shared_ptr<core::dbus::Object> object;

    struct
    {
        std::shared_ptr<core::dbus::Signal<ModemAdded, ModemAdded::ArgumentType>> modem_added;
        std::shared_ptr<core::dbus::Signal<ModemRemoved, ModemRemoved::ArgumentType>> modem_removed;
    } signals;
};
};
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_OFONO_H
