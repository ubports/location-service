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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_ACCURACY_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_ACCURACY_H_

#include <ostream>

namespace com
{
namespace ubuntu
{
namespace location
{
enum class AccuracyLevel
{
    country,
    region,
    locality,
    postalcode,
    street,
    detailed,
    
    best = detailed,
    worst = country
};

template<typename T>
struct AccuracyTraits
{
    static AccuracyLevel classify(const T& instance);
    static T best();
    static T worst();
};

template<typename T>
struct Accuracy
{
    typedef typename T::Unit Unit;
    typedef typename T::Quantity Quantity;

    static Accuracy<T> best() { return AccuracyTraits<T>::best(); }
    static Accuracy<T> worst() { return AccuracyTraits<T>::worst(); }

    AccuracyLevel classify() const
    {
        return AccuracyTraits<T>::classify(value);
    }

    T value;
};

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const Accuracy<T>& update)
{
    out << "Accuracy(" << update.value << ")";
    return out;
}

}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_ACCURACY_H_
