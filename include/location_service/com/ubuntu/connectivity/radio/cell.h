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
#ifndef LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_CELL_H_
#define LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_CELL_H_

#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace connectivity
{
namespace radio
{
template<int min, int max>
struct BoundedInteger
{
    static_assert(min < max, "min >= max");

    explicit BoundedInteger(int value) : value(value)
    {
        if (value < min || value > max)
            throw std::runtime_error(
                    std::to_string(value) + " is not in " + "[" +
                    std::to_string(min) + ", " + std::to_string(max) + "]");
    }

    inline operator int() const
    {
        return value;
    }

    int value;
};

class Cell
{
public:
    enum class RadioType
    {
        unknown,
        gsm,
        umts,
        cdma,
        lte
    };

    struct Gsm
    {
        BoundedInteger<0,999> mobile_country_code;
        BoundedInteger<0,999> mobile_network_code;
        BoundedInteger<0,65535> location_area_code;
        BoundedInteger<0,65535> id;
        BoundedInteger<-113,-51> received_signal_strength;
        BoundedInteger<0,31> arbitrary_strength_unit;
        BoundedInteger<0,63> timing_advance;
    };

    struct Umts
    {
        BoundedInteger<0,999> mobile_country_code;
        BoundedInteger<0,999> mobile_network_code;
        BoundedInteger<0,65535> location_area_code;
        BoundedInteger<0,268435455> id;
        BoundedInteger<-121,-25> received_signal_strength;
        BoundedInteger<-5,91> arbitrary_strength_unit;
    };

    struct Cdma
    {
        BoundedInteger<0,999> mobile_country_code;
        BoundedInteger<0,32767> mobile_network_code;
        BoundedInteger<0,65535> location_area_code;
        BoundedInteger<0,65535> id;
        BoundedInteger<-100,-75> received_signal_strength;
        BoundedInteger<1,16> arbitrary_strength_unit;
    };

    struct Lte
    {
        BoundedInteger<0,999> mobile_country_code;
        BoundedInteger<0,32767> mobile_network_code;
        BoundedInteger<0,65535> location_area_code;
        BoundedInteger<0,268435455> id;
        BoundedInteger<0,503> physical_id;
        BoundedInteger<-137,-45> received_signal_strength;
        BoundedInteger<0,95> arbitrary_strength_unit;
        BoundedInteger<0,63> timing_advance;
    };

    explicit Cell(const Gsm& gsm)
        : radio_type(RadioType::gsm), detail{gsm}
    {
    }

    explicit Cell(const Umts& umts)
        : radio_type(RadioType::umts), detail{umts}
    {
    }

    explicit Cell(const Cdma& cdma)
        : radio_type(RadioType::cdma), detail{cdma}
    {
    }

    explicit Cell(const Lte& lte)
        : radio_type(RadioType::lte), detail{lte}
    {
    }

    RadioType type() const
    {
        return radio_type;
    }

    const Gsm& gsm() const
    {
        if (radio_type != RadioType::gsm)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.gsm;
    }

    const Umts& umts() const
    {
        if (radio_type != RadioType::umts)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.umts;
    }

    const Cdma& cdma() const
    {
        if (radio_type != RadioType::cdma)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.cdma;
    }

    const Lte& lte() const
    {
        if (radio_type != RadioType::lte)
            throw std::runtime_error("Bad access to unset network type.");

        return detail.lte;
    }

private:
    RadioType radio_type;

    union Detail
    {
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

        Gsm gsm;
        Umts umts;
        Cdma cdma;
        Lte lte;
    } detail;
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_CONNECTIVITY_RADIO_CELL_H_

