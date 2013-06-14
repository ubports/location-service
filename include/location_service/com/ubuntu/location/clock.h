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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_

#include <chrono>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Clock
{
    typedef std::chrono::high_resolution_clock::duration Duration;
    typedef std::chrono::high_resolution_clock::time_point Timestamp;

    static inline Timestamp now()
    {
        return std::chrono::high_resolution_clock::now();
    }
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_