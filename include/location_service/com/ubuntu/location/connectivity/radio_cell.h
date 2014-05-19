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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_RADIO_CELL_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_RADIO_CELL_H_

#include <com/ubuntu/location/connectivity/bounded_integer.h>

#include <limits>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace connectivity
{
class RadioCell
{
public:

    enum class Type
    {
        unknown,
        gsm,
        umts,
        lte
    };

    struct Mcc {};
    struct Mnc {};
    struct Lac {};
    struct Tac {};
    struct Id {};
    struct Psc {};
    struct Pid {};
    struct Rss {};
    struct Asu {};
    struct Ta {};

    template<int min, int max, int invalid = min-1>
    using MobileCountryCode = BoundedInteger<Mcc, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using MobileNetworkCode = BoundedInteger<Mnc, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using LocationAreaCode = BoundedInteger<Lac, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using TrackingAreaCode = BoundedInteger<Tac, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using CellId = BoundedInteger<Id, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using PrimaryScramblingCode = BoundedInteger<Psc, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using PhysicalId = BoundedInteger<Pid, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using ReceivedSignalStrength = BoundedInteger<Rss, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using ArbitraryStrengthUnit = BoundedInteger<Asu, min, max, invalid>;
    template<int min, int max, int invalid = min-1>
    using TimingAdvance = BoundedInteger<Ta, min, max, invalid>;

    struct Gsm
    {
        /** 3-digit Mobile Country Code, 0..999, INT_MAX if unknown */
        typedef MobileCountryCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MCC;
        /** 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown */
        typedef MobileNetworkCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MNC;
        /** 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
        typedef LocationAreaCode
        <
            0,
            65535,
            std::numeric_limits<int>::max()
        > LAC;
        /** 16-bit GSM Cell Identity described in TS 27.007, 0..65535, INT_MAX if unknown  */
        typedef CellId
        <
            0,
            65535,
            std::numeric_limits<int>::max()
        > ID;

        /** Valid values are (0-31, 99) as defined in TS 27.007 8.5 */
        typedef ArbitraryStrengthUnit
        <
            0,
            31,
            99
        > SignalStrength;

        bool operator==(const Gsm& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    strength == rhs.strength;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Gsm& gsm)
        {
            out << "("
                << "mcc: " << gsm.mobile_country_code << ", "
                << "mnc: " << gsm.mobile_network_code << ", "
                << "lac: " << gsm.location_area_code << ", "
                << "id: " << gsm.id << ", "
                << "asu: " << gsm.strength << ")";

            return out;
        }

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        SignalStrength strength;
    };

    struct Umts
    {
        /** 3-digit Mobile Country Code, 0..999, INT_MAX if unknown */
        typedef MobileCountryCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MCC;
        /** 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown */
        typedef MobileNetworkCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MNC;
        /** 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
        typedef LocationAreaCode
        <
            0,
            65535,
            std::numeric_limits<int>::max()
        > LAC;
        /** 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknown  */
        typedef CellId
        <
            0,
            268435455,
            std::numeric_limits<int>::max()
        > ID;
        /** 9-bit UMTS Primary Scrambling Code described in TS 25.331, 0..511, INT_MAX if unknown */
        typedef PrimaryScramblingCode
        <
            0,
            511,
            std::numeric_limits<int>::max()
        > PSC;
        /** Valid values are (0-31, 99) as defined in TS 27.007 8.5 */
        typedef ArbitraryStrengthUnit
        <
            0,
            31,
            99
        > SignalStrength;

        bool operator==(const Umts& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    primary_scrambling_code == rhs.primary_scrambling_code &&
                    strength == rhs.strength;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Umts& umts)
        {
            out << "("
                << "mcc: " << umts.mobile_country_code << ", "
                << "mnc: " << umts.mobile_network_code << ", "
                << "lac: " << umts.location_area_code << ", "
                << "id: " << umts.id << ", "
                << "psc: " << umts.primary_scrambling_code << ", "
                << "asu: " << umts.strength << ")";

            return out;
        }
        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        PSC primary_scrambling_code;
        SignalStrength strength;
    };

    struct Lte
    {
        /** 3-digit Mobile Country Code, 0..999, INT_MAX if unknown */
        typedef MobileCountryCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MCC;
        /** 2 or 3-digit Mobile Network Code, 0..999, INT_MAX if unknown */
        typedef MobileNetworkCode
        <
            0,
            999,
            std::numeric_limits<int>::max()
        > MNC;
        /** 16-bit Location Area Code, 0..65535, INT_MAX if unknown  */
        typedef LocationAreaCode
        <
            0,
            65535,
            std::numeric_limits<int>::max()
        > LAC;
        /** 28-bit Cell Identity described in TS 25.331, 0..268435455, INT_MAX if unknown  */
        typedef CellId
        <
            0,
            268435455,
            std::numeric_limits<int>::max()
        > ID;
        /** Physical cell id, 0..503, INT_MAX if unknown */
        typedef PhysicalId
        <
            0,
            503,
            std::numeric_limits<int>::max()
        > PID;
        /** Valid values are (0-31, 99) as defined in TS 27.007 8.5 */
        typedef ArbitraryStrengthUnit
        <
            0,
            31,
            99
        > SignalStrength;

        bool operator==(const Lte& rhs) const
        {
            return mobile_country_code == rhs.mobile_country_code &&
                    mobile_network_code == rhs.mobile_network_code &&
                    location_area_code == rhs.location_area_code &&
                    id == rhs.id &&
                    physical_id == rhs.physical_id &&
                    strength == rhs.strength;
        }

        inline friend std::ostream& operator<<(std::ostream& out, const Lte& lte)
        {
            out << "("
                << "mcc: " << lte.mobile_country_code << ", "
                << "mnc: " << lte.mobile_network_code << ", "
                << "lac: " << lte.location_area_code << ", "
                << "id: " << lte.id << ", "
                << "id: " << lte.physical_id << ", "
                << "asu: " << lte.strength << ")";

            return out;
        }

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        PID physical_id;
        SignalStrength strength;
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

    explicit RadioCell(const Lte& lte)
        : radio_type(Type::lte), detail{lte}
    {
    }

    RadioCell(const RadioCell& rhs) : radio_type(rhs.radio_type)
    {
        switch(radio_type)
        {
        case Type::gsm: detail.gsm = rhs.detail.gsm; break;
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
        case RadioCell::Type::umts: out << cell.detail.umts; break;
        case RadioCell::Type::lte: out << cell.detail.lte; break;
        case RadioCell::Type::unknown: break;
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

        inline explicit Detail(const Lte& lte) : lte(lte)
        {
        }
        None none;
        Gsm gsm;
        Umts umts;
        Lte lte;
    } detail;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_RADIO_CELL_H_

