/*
 * Copyright © 2012-2014 Canonical Ltd.
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
#ifndef CACHED_WIRELESS_NETWORK_H_
#define CACHED_WIRELESS_NETWORK_H_

#include <com/ubuntu/location/connectivity/wireless_network.h>

#include "nm.h"

namespace
{
std::string utf8_ssid_to_string(const org::freedesktop::NetworkManager::AccessPoint::Ssid::ValueType& ssid)
{
    return std::string(ssid.begin(), ssid.end());
}

com::ubuntu::location::connectivity::WirelessNetwork::Mode
wifi_mode_from_ap_mode(org::freedesktop::NetworkManager::AccessPoint::Mode::ValueType value)
{
    com::ubuntu::location::connectivity::WirelessNetwork::Mode mode
    {
        com::ubuntu::location::connectivity::WirelessNetwork::Mode::unknown
    };

    switch (value)
    {
    case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::unknown:
        mode = com::ubuntu::location::connectivity::WirelessNetwork::Mode::unknown;
        break;
    case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::adhoc:
        mode = com::ubuntu::location::connectivity::WirelessNetwork::Mode::adhoc;
        break;
    case org::freedesktop::NetworkManager::AccessPoint::Mode::Value::infra:
        mode = com::ubuntu::location::connectivity::WirelessNetwork::Mode::infrastructure;
        break;
    }

    return mode;
}

struct CachedWirelessNetwork : public com::ubuntu::location::connectivity::WirelessNetwork
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
        ssid_ = utf8_ssid_to_string(access_point_.ssid->get());
        mode_ = wifi_mode_from_ap_mode(access_point_.mode->get());
        frequency_ = com::ubuntu::location::connectivity::WirelessNetwork::Frequency
        {
            access_point_.frequency->get()
        };
        signal_strength_ = com::ubuntu::location::connectivity::WirelessNetwork::SignalStrength
        {
            int(access_point_.strength->get())
        };

        // Wire up all the connections
        access_point_.properties_changed->connect([this](const std::map<std::string, core::dbus::types::Variant>& dict)
        {
            on_access_point_properties_changed(dict);
        });
    }

    void on_access_point_properties_changed(const std::map<std::string, core::dbus::types::Variant>& dict)
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
                    thiz.ssid_ = utf8_ssid_to_string(value.as<org::freedesktop::NetworkManager::AccessPoint::Ssid::ValueType>());
                }
            },
            {
                org::freedesktop::NetworkManager::AccessPoint::Strength::name(),
                [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                {
                    thiz.signal_strength_ = com::ubuntu::location::connectivity::WirelessNetwork::SignalStrength
                    {
                        value.as<org::freedesktop::NetworkManager::AccessPoint::Strength::ValueType>()
                    };
                }
            },
            {
                org::freedesktop::NetworkManager::AccessPoint::Frequency::name(),
                [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                {
                    thiz.frequency_ = com::ubuntu::location::connectivity::WirelessNetwork::Frequency
                    {
                        value.as<org::freedesktop::NetworkManager::AccessPoint::Frequency::ValueType>()
                    };
                }
            },
            {
                org::freedesktop::NetworkManager::AccessPoint::Mode::name(),
                [](CachedWirelessNetwork& thiz, const core::dbus::types::Variant& value)
                {
                    thiz.mode_ = wifi_mode_from_ap_mode(value.as<org::freedesktop::NetworkManager::AccessPoint::Mode::ValueType>());
                }
            }
        };

        for (const auto& pair : dict)
        {
            VLOG(1) << "Properties on access point " << ssid_.get() << " changed: \n"
                    << "  " << pair.first;

            // We do not treat failing property updates as fatal but instead just
            // log the issue for later analysis.
            try
            {
                if (lut.count(pair.first) > 0) lut.at(pair.first)(*this, pair.second);
            } catch (const std::exception& e)
            {
                LOG(WARNING) << "Exception while updating state for property change: " << pair.first;
            }
        }
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
}

#endif // CACHED_WIRELESS_NETWORK_H_
