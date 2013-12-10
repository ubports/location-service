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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_

#include <com/ubuntu/location/clock.h>

#include <ostream>

namespace com
{
namespace ubuntu
{
namespace location
{
/**
 * @brief Templated class that wraps a value and timestamp.
 * @tparam T The contained value.
 */
template<typename T>
struct Update
{
    /**
      * @brief Constructs a valid update with the given value and timestamp.
      * @param [in] value The value delivered with this update.
      * @param [in] when The timestamp when the value was measured.
      */
    inline Update(const T& value = T{},
           const Clock::Timestamp& when = Clock::now())
        : value{value}, when{when}
    {
    }

    /**
     * @brief operator == checks if two updates are equal.
     * @param [in] rhs The update to check against.
     * @return true iff this instance equals rhs.
     */
    inline bool operator==(const Update<T>& rhs) const
    {
        return value == rhs.value && when == rhs.when;
    }

    /**
     * @brief operator != checks if two updates are unequal.
     * @param [in] rhs The update to check against.
     * @return true iff this instance does not equal rhs.
     */
    inline bool operator!=(const Update<T>& rhs) const
    {
        return !(value == rhs.value && when == rhs.when);
    }

    /** The value delivered with this update. */
    T value;

    /** Time when the updated value was measured. */
    Clock::Timestamp when = Clock::beginning_of_time();
};

/**
 * @brief Pretty-prints the update to the provided output stream.
 * @param out The stream to write to.
 * @param update The value to be printed.
 */
template<typename T>
inline std::ostream& operator<<(std::ostream& out, const Update<T>& update)
{
    out << "Update(" << update.value << ", " << update.when.time_since_epoch().count() << ")";
    return out;
}

}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_
