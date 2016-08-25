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
#ifndef LOCATION_EVENTS_ALL_H_
#define LOCATION_EVENTS_ALL_H_

#include <location/events/reference_position_updated.h>
#include <location/events/wifi_and_cell_id_reporting_state_changed.h>

#include <boost/variant.hpp>

namespace location
{
namespace events
{
/// @brief Undefined tags the unknown event and allows us to
/// escape the "no default ctor" issue.
struct Undefined{};

/// @brief All captures all known events and allows us to serialize/transport
/// them easily without the need to extend location::Event with specific serialization
/// functionality.
typedef boost::variant
<
    Undefined,
    ReferencePositionUpdated,
    WifiAndCellIdReportingStateChanged
> All;
}
}

#endif // LOCATION_EVENTS_ALL_H_
