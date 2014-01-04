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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_

#include <core/property.h>

#include <string>
#include <vector>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace connectivity
{
template<int min, int max, int domain = 0>
class BoundedInteger
{
public:
    static_assert(min < max, "min >= max");

    explicit BoundedInteger(int value = (min + (max-min)/2)) : value(value)
    {
        if (value < min || value > max)
            throw std::runtime_error(
                    std::to_string(value) + " is not in " + "[" +
                    std::to_string(min) + ", " + std::to_string(max) + "]");
    }

    BoundedInteger(const BoundedInteger<min, max, domain>& rhs) : value(rhs.value)
    {
    }

    BoundedInteger<min, max, domain>& operator=(const BoundedInteger<min, max, domain>& rhs)
    {
        value = rhs.value;
        return *this;
    }

    bool operator==(const BoundedInteger<min, max, domain>& rhs) const
    {
        return value == rhs.value;
    }

    inline operator int() const
    {
        return value;
    }

    inline int get() const
    {
        return value;
    }

    inline void set(int new_value)
    {
        if (new_value < min || new_value > max)
            throw std::runtime_error(
                    std::to_string(new_value) + " is not in " + "[" +
                    std::to_string(min) + ", " + std::to_string(max) + "]");

        value = new_value;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const BoundedInteger<min, max, domain>& bi)
    {
        return out << bi.value;
    }

private:
    int value;
};

class RadioCell
{
public:
    enum class Type
    {
        unknown,
        gsm,
        umts,
        cdma,
        lte
    };

    enum class Domain
    {
        mcc,
        mnc,
        lac,
        id,
        pid,
        rss,
        asu,
        ta
    };

    template<int min, int max>
    using MobileCountryCode = BoundedInteger<min, max, static_cast<int>(Domain::mcc)>;
    template<int min, int max>
    using MobileNetworkCode = BoundedInteger<min, max, static_cast<int>(Domain::mnc)>;
    template<int min, int max>
    using LocationAreaCode = BoundedInteger<min, max, static_cast<int>(Domain::lac)>;
    template<int min, int max>
    using CellId = BoundedInteger<min, max, static_cast<int>(Domain::id)>;
    template<int min, int max>
    using PhysicalId = BoundedInteger<min, max, static_cast<int>(Domain::pid)>;
    template<int min, int max>
    using ReceivedSignalStrength = BoundedInteger<min, max, static_cast<int>(Domain::rss)>;
    template<int min, int max>
    using ArbitraryStrengthUnit = BoundedInteger<min, max, static_cast<int>(Domain::asu)>;
    template<int min, int max>
    using TimingAdvance = BoundedInteger<min, max, static_cast<int>(Domain::ta)>;

    struct Gsm
    {
        typedef MobileCountryCode<0,999> MCC;
        typedef MobileNetworkCode<0,999> MNC;
        typedef LocationAreaCode<0,65535> LAC;
        typedef CellId<0,65535> ID;
        typedef ReceivedSignalStrength<-113,-51> RSS;
        typedef ArbitraryStrengthUnit<0,31> ASU;
        typedef TimingAdvance<0,63> TA;

        bool operator==(const Gsm& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    received_signal_strength == rhs.received_signal_strength &&
                    arbitrary_strength_unit == rhs.arbitrary_strength_unit &&
                    timing_advance == rhs.timing_advance;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Gsm& gsm)
        {
            out << "("
                << "mcc: " << gsm.mobile_country_code << ", "
                << "mnc: " << gsm.mobile_network_code << ", "
                << "lac: " << gsm.location_area_code << ", "
                << "id: " << gsm.id << ", "
                << "rss: " << gsm.received_signal_strength << ", "
                << "asu: " << gsm.arbitrary_strength_unit << ", "
                << "ta: " << gsm.timing_advance << ", ";

            return out;
        }

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        RSS received_signal_strength;
        ASU arbitrary_strength_unit;
        TA timing_advance;
    };

    struct Umts
    {
        typedef MobileCountryCode<0,999> MCC;
        typedef MobileNetworkCode<0,999> MNC;
        typedef LocationAreaCode<0,65535> LAC;
        typedef CellId<0,268435455> ID;
        typedef ReceivedSignalStrength<-121,-25> RSS;
        typedef ArbitraryStrengthUnit<-5,91> ASU;

        bool operator==(const Umts& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    received_signal_strength == rhs.received_signal_strength &&
                    arbitrary_strength_unit == rhs.arbitrary_strength_unit;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Umts& umts)
        {
            out << "("
                << "mcc: " << umts.mobile_country_code << ", "
                << "mnc: " << umts.mobile_network_code << ", "
                << "lac: " << umts.location_area_code << ", "
                << "id: " << umts.id << ", "
                << "rss: " << umts.received_signal_strength << ", "
                << "asu: " << umts.arbitrary_strength_unit << ", ";

            return out;
        }
        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        RSS received_signal_strength;
        ASU arbitrary_strength_unit;
    };

    struct Cdma
    {
        typedef MobileCountryCode<0,999> MCC;
        typedef MobileNetworkCode<0,32767> MNC;
        typedef LocationAreaCode<0,65535> LAC;
        typedef CellId<0,65535> ID;
        typedef ReceivedSignalStrength<-100,-75> RSS;
        typedef ArbitraryStrengthUnit<1,16> ASU;

        bool operator==(const Cdma& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    received_signal_strength == rhs.received_signal_strength &&
                    arbitrary_strength_unit == rhs.arbitrary_strength_unit;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Cdma& cdma)
        {
            out << "("
                << "mcc: " << cdma.mobile_country_code << ", "
                << "mnc: " << cdma.mobile_network_code << ", "
                << "lac: " << cdma.location_area_code << ", "
                << "id: " << cdma.id << ", "
                << "rss: " << cdma.received_signal_strength << ", "
                << "asu: " << cdma.arbitrary_strength_unit << ", ";

            return out;
        }

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        RSS received_signal_strength;
        ASU arbitrary_strength_unit;
    };

    struct Lte
    {
        typedef MobileCountryCode<0,999> MCC;
        typedef MobileNetworkCode<0,32767> MNC;
        typedef LocationAreaCode<0,65535> LAC;
        typedef CellId<0,268435455> ID;
        typedef PhysicalId<0,503> PID;
        typedef ReceivedSignalStrength<-137,-45> RSS;
        typedef ArbitraryStrengthUnit<0,95> ASU;
        typedef TimingAdvance<0,63> TA;

        bool operator==(const Lte& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    physical_id == rhs.physical_id &&
                    received_signal_strength == rhs.received_signal_strength &&
                    arbitrary_strength_unit == rhs.arbitrary_strength_unit &&
                    timing_advance == rhs.timing_advance;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Lte& lte)
        {
            out << "("
                << "mcc: " << lte.mobile_country_code << ", "
                << "mnc: " << lte.mobile_network_code << ", "
                << "lac: " << lte.location_area_code << ", "
                << "id: " << lte.id << ", "
                << "id: " << lte.physical_id << ", "
                << "rss: " << lte.received_signal_strength << ", "
                << "asu: " << lte.arbitrary_strength_unit << ", "
                << "ta: " << lte.timing_advance << ", ";

            return out;
        }

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        PID physical_id;
        RSS received_signal_strength;
        ASU arbitrary_strength_unit;
        TA timing_advance;
    };

    RadioCell() : radio_type(Type::gsm), detail{Gsm()}
    {
    }

    explicit RadioCell(const Gsm& gsm)
        : radio_type(Type::gsm), detail{gsm}
    {
    }

    explicit RadioCell(const Umts& umts)
        : radio_type(Type::umts), detail{umts}
    {
    }

    explicit RadioCell(const Cdma& cdma)
        : radio_type(Type::cdma), detail{cdma}
    {
    }

    explicit RadioCell(const Lte& lte)
        : radio_type(Type::lte), detail{lte}
    {
    }

    RadioCell(const RadioCell& rhs) : radio_type(rhs.radio_type)
    {
        switch(radio_type)
        {
        case Type::gsm: detail.gsm = rhs.detail.gsm; break;
        case Type::cdma: detail.cdma = rhs.detail.cdma; break;
        case Type::umts: detail.umts = rhs.detail.umts; break;
        case Type::lte: detail.lte = rhs.detail.lte; break;
        case Type::unknown: break;
        }
    }

    RadioCell& operator=(const RadioCell& rhs)
    {
        radio_type = rhs.radio_type;
        switch(radio_type)
        {
        case Type::gsm: detail.gsm = rhs.detail.gsm; break;
        case Type::cdma: detail.cdma = rhs.detail.cdma; break;
        case Type::umts: detail.umts = rhs.detail.umts; break;
        case Type::lte: detail.lte = rhs.detail.lte; break;
        case Type::unknown: break;
        }

        return *this;
    }

    bool operator==(const RadioCell& rhs) const
    {
        if (radio_type != rhs.radio_type)
            return false;

        switch(radio_type)
        {
        case Type::gsm: return detail.gsm == rhs.detail.gsm;
        case Type::cdma: return detail.cdma == rhs.detail.cdma;
        case Type::umts: return detail.umts == rhs.detail.umts;
        case Type::lte: return detail.lte == rhs.detail.lte;
        default: return true;
        }

        return false;
    }

    Type type() const
    {
        return radio_type;
    }

    const Gsm& gsm() const
    {
        if (radio_type != Type::gsm)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.gsm;
    }

    const Umts& umts() const
    {
        if (radio_type != Type::umts)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.umts;
    }

    const Cdma& cdma() const
    {
        if (radio_type != Type::cdma)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.cdma;
    }

    const Lte& lte() const
    {
        if (radio_type != Type::lte)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.lte;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const RadioCell& cell)
    {
        switch (cell.radio_type)
        {
        case RadioCell::Type::gsm: out << cell.detail.gsm; break;
        case RadioCell::Type::cdma: out << cell.detail.cdma; break;
        case RadioCell::Type::umts: out << cell.detail.umts; break;
        case RadioCell::Type::lte: out << cell.detail.lte; break;
        }

        return out;
    }

private:
    Type radio_type;

    struct None {};
    union Detail
    {
        inline Detail() : none(None{})
        {
        }

        inline explicit Detail(const Gsm& gsm) : gsm(gsm)
        {
        }

        inline explicit Detail(const Umts& umts) : umts(umts)
        {
        }

        inline explicit Detail(const Cdma& cdma) : cdma(cdma)
        {
        }

        inline explicit Detail(const Lte& lte) : lte(lte)
        {
        }
        None none;
        Gsm gsm;
        Umts umts;
        Cdma cdma;
        Lte lte;
    } detail;
};

struct WirelessNetwork
{
    enum class Domain
    {
        frequency,
        channel
    };

    typedef BoundedInteger<2412, 5825, static_cast<int>(Domain::frequency)> Frequency;
    typedef BoundedInteger<1, 165, static_cast<int>(Domain::channel)> Channel;

    bool operator==(const WirelessNetwork& rhs) const
    {
        return bssid == rhs.bssid &&
               frequency == rhs.frequency &&
               channel == rhs.channel &&
               snr == rhs.snr;
    }

    friend std::ostream& operator<<(std::ostream& out, const WirelessNetwork& wifi)
    {
        return out << "(" << wifi.bssid << ", " << wifi.frequency << ", " << wifi.channel << ", " << wifi.snr << ")";
    }

    std::string bssid; ///< The BSSID of the network.
    Frequency frequency; ///< Frequency of the network/AP.
    Channel channel; ///< Channel of the network/AP.
    float snr; ///< Signal-noise ratio of the specific network.
};

/**
 * @brief The Manager class encapsulates access to network/radio information
 */
class Manager
{
public:
    struct Errors
    {
        Errors() = delete;
        /**
         * @brief The ConnectivityManagementNotSupported struct is thrown if underlying
         * platform does not provide support for connectivity mgmt.
         */
        struct ConnectivityManagementNotSupported : public std::runtime_error
        {
            ConnectivityManagementNotSupported()
                : std::runtime_error(
                      "Underlying platform does not provide support for connectivity mgmt.")
            {
            }
        };
    };

    Manager(const Manager& rhs) = delete;
    virtual ~Manager() = default;

    Manager& operator=(const Manager& rhs) = delete;
    bool operator==(const Manager& rhs) const = delete;

    /**
     * @brief All wireless networks visible to the device.
     * @return A getable/observable property carrying the visible wireless networks.
     */
    virtual const core::Property<std::vector<WirelessNetwork>>& visible_wireless_networks() = 0;

    /**
     * @brief All radio cells visible to the device.
     * @return A getable/observable property carrying the visible radio cells.
     */
    virtual const core::Property<std::vector<RadioCell>>& visible_radio_cells() = 0;

protected:
    Manager() = default;
};

/**
 * @brief Provides access to a platform-specific implementation/instance of a connectivity manager.
 * @throw Manager::Errors::ConnectivityManagementNotSupported.
 * @return An instance of a connectivity manager.
 */
const std::shared_ptr<Manager>& platform_default_manager();
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_MANAGER_H_
