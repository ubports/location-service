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
#ifndef LOCATION_CLOCK_H_
#define LOCATION_CLOCK_H_

#include <location/visibility.h>

#include <chrono>

namespace location
{
/**
 * @brief Defines the timebase of the location service.
 */
struct LOCATION_DLL_PUBLIC Clock
{
    /** @brief The underlying clock we are assuming for all time-stamping purposes. */
    typedef std::chrono::high_resolution_clock Type;

    /**
     * @brief Duration type of the location service clock.
     */
    typedef std::chrono::high_resolution_clock::duration Duration;

    /**
     * @brief Timestamp type of the location service clock.
     */
    typedef std::chrono::high_resolution_clock::time_point Timestamp;

    /**
     * @brief Samples a timestamp from the clock.
     * @return The current time.
     */
    static inline Timestamp now()
    {
        return std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Samples a timestamp from the clock.
     * @return The minimum point in time representable by this clock.
     */
    static inline Timestamp beginning_of_time()
    {
        return std::chrono::high_resolution_clock::time_point::min();
    }
};
}

#endif // LOCATION_CLOCK_H_
