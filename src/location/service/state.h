/*
 * Copyright © 2016 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_STATE_H_
#define LOCATION_SERVICE_STATE_H_

#include <iosfwd>

namespace location { namespace service
{
/// @brief State enumerates the known states of the service.
enum class State
{
    disabled, ///< The service has been disabled by the user.
    enabled,  ///< The service is enabled but not actively carrying out positioning.
    active    ///< The service is actively trying to determine the position of the device.
};

/// @brief operator== returns true if lhs == rhs.
bool operator==(State lhs, State rhs);
/// @brief operator != returns true if !(lhs == rhs).
bool operator!=(State lhs, State rhs);

/// @brief operator<< inserts state into the out.
std::ostream& operator<<(std::ostream& out, State state);
/// @brief operator>> extracts state from in.
std::istream& operator>>(std::istream& in, State& state);
}}
#endif // LOCATION_SERVICE_STATE_H_
