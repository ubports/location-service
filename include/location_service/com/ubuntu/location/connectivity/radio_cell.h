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
/** @brief Models a radio cell that one of the modems in the system is connected to. */
class RadioCell
{
public:

    /** @brief Enumerates the known technologies. */
    enum class Type
    {
        unknown,
        gsm,
        umts,
        lte
    };

    /** @cond */
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
    /** @endcond */

    /** @brief Models a GSM radio cell. */
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

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        SignalStrength strength;
    };

    /** @brief Models a UMTS radio cell. */
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

        MCC mobile_country_code;
        MNC mobile_network_code;
        LAC location_area_code;
        ID id;
        PSC primary_scrambling_code;
        SignalStrength strength;
    };

    /** @brief Models an LTE radio cell. */
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
        /** 16-bit Tracking Area Code, 0..65535, INT_MAX if unknown  */
        typedef TrackingAreaCode
        <
            0,
            65535,
            std::numeric_limits<int>::max()
        > TAC;
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

        MCC mobile_country_code;
        MNC mobile_network_code;
        TAC tracking_area_code;
        ID id;
        PID physical_id;
        SignalStrength strength;
    };

    RadioCell();
    explicit RadioCell(const Gsm& gsm);
    explicit RadioCell(const Umts& umts);
    explicit RadioCell(const Lte& lte);

    RadioCell(const RadioCell& rhs);
    RadioCell& operator=(const RadioCell& rhs);

    /** @brief Returns the type of the radio cell. */
    Type type() const;

    /** @brief Returns GSM-specific details or throws std::runtime_error if this is not a GSM radiocell. */
    const Gsm& gsm() const;

    /** @brief Returns UMTS-specific details or throws std::runtime_error if this is not a UMTS radiocell. */
    const Umts& umts() const;

    /** @brief Returns LTE-specific details or throws std::runtime_error if this is not an LTE radiocell. */
    const Lte& lte() const;

private:
    /** @cond */
    Type radio_type;

    struct None {};

    union Detail
    {
        Detail();
        Detail(const Gsm& gsm);
        Detail(const Umts& umts);
        Detail(const Lte& lte);

        None none;
        Gsm gsm;
        Umts umts;
        Lte lte;
    } detail;
    /** @endcond */
};

/** @brief Returns true iff lhs equals rhs. */
bool operator==(const RadioCell::Gsm& lhs, const RadioCell::Gsm& rhs);

/** @brief Pretty-prints the given gsm details to the given output stream. */
std::ostream& operator<<(std::ostream& out, const RadioCell::Gsm& gsm);

/** @brief Returns true iff lhs equals rhs. */
bool operator==(const RadioCell::Umts& lhs, const RadioCell::Umts& rhs);

/** @brief Pretty-prints the given umts details to the given output stream. */
std::ostream& operator<<(std::ostream& out, const RadioCell::Umts& umts);

/** @brief Returns true iff lhs equals rhs. */
bool operator==(const RadioCell::Lte& lhs, const RadioCell::Lte& rhs);

/** @brief Pretty-prints the given gsm details to the given output stream. */
std::ostream& operator<<(std::ostream& out, const RadioCell::Lte& lte);

/** @brief Returns true iff lhs equals rhs. */
bool operator==(const RadioCell& lhs, const RadioCell& rhs);

/** @brief Pretty-prints the given cell to the given output stream. */
std::ostream& operator<<(std::ostream& out, const RadioCell& cell);
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CONNECTIVITY_RADIO_CELL_H_

