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

#ifndef LOCATION_UTIL_FLAGS_H_
#define LOCATION_UTIL_FLAGS_H_

#include <type_traits>

namespace location
{
namespace util
{
/// @brief Flags<T> helps in handling enum classes as bitfields.
template<typename T>
struct Flags
{
    // Bail out if T is not an enum.
    static_assert(std::is_enum<T>::value, "T must be an enum (class)");

    /// @brief has returns true if flag is set in flags.
    static bool has(T flags, T flag)
    {
        return (flags & flag) == flag;
    }
};

namespace flags
{
template<typename T>
inline bool has(T set, T flag)
{
    return Flags<T>::has(set, flag);
}
}
}
}

#endif // LOCATION_UTIL_FLAGS_H_
