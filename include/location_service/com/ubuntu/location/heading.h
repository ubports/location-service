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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_HEADING_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_HEADING_H_

#include "com/ubuntu/location/accuracy.h"
#include "com/ubuntu/location/units/units.h"

#include <limits>
#include <ostream>
#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Heading
{
    typedef units::PlaneAngle Unit;
    typedef units::Quantity<Unit> Quantity;

    static const Quantity& min()
    {
        static const auto instance = Heading::Quantity::from_value(0.);
        return instance;
    }
    static const Quantity& max()
    {
        static const auto instance = Heading::Quantity::from_value(360.);
        return instance;
    }

    Heading(const Quantity& value = Quantity()) : value(value)
    {
        if (value < min())
            throw std::out_of_range("");
        if (value > max())
            throw std::out_of_range("");
    }

    bool operator==(const Heading& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const Heading& rhs) const
    {
        return value != rhs.value;
    }

    Quantity value;
};

inline std::ostream& operator<<(std::ostream& out, const Heading& heading)
{
    out << "Heading(" << heading.value << ")";
    return out;
}

template<>
struct AccuracyTraits<Heading>
{
    static AccuracyLevel classify(const Heading& h)
    {
        static const auto half = 0.5 * Heading::max();
        if(h.value > half)
            return AccuracyLevel::worst;

        if(h.value < half)
            return AccuracyLevel::best;

        return AccuracyLevel::worst;
    }

    static Accuracy<Heading> best()
    {
        return Accuracy<Heading>{Heading{Heading::min()}};
    }

    static Accuracy<Heading> worst()
    {
        return Accuracy<Heading>{Heading{Heading::max()}};
    }
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_HEADING_H_
