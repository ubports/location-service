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

#ifndef LOCATION_EVENTS_WIFI_AND_CELL_ID_REPORTING_STATE_CHANGED_H_
#define LOCATION_EVENTS_WIFI_AND_CELL_ID_REPORTING_STATE_CHANGED_H_

#include <location/event.h>

#include <location/wifi_and_cell_reporting_state.h>

namespace location
{
namespace events
{
/// @brief WifiAndCellIdReportingStateChanged is sent if the WifiAndCellIdReportingState changed.
class WifiAndCellIdReportingStateChanged : public Event
{
public:
    /// @brief WifiAndCellIdReportingStateChanged initializes a new instance with new_state.
    WifiAndCellIdReportingStateChanged(WifiAndCellIdReportingState new_state);
    /// @brief WifiAndCellIdReportingStateChanged initializes a new instance from rhs.
    WifiAndCellIdReportingStateChanged(const WifiAndCellIdReportingStateChanged& rhs);
    /// @brief operator= assigns rhs to this instance.
    WifiAndCellIdReportingStateChanged& operator=(const WifiAndCellIdReportingStateChanged& rhs);

    /// @brief new_state returns the new state.
    WifiAndCellIdReportingState new_state() const;

    // From Bus::Message
    Type type() const override;

private:
    WifiAndCellIdReportingState new_state_;
};
}
template<>
struct TypeOf<events::WifiAndCellIdReportingStateChanged>
{
    static Event::Type query();
};
}

#endif // LOCATION_EVENTS_WIFI_AND_CELL_ID_REPORTING_STATE_CHANGED_H_
