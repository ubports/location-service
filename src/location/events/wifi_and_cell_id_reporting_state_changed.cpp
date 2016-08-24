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

#include <location/events/wifi_and_cell_id_reporting_state_changed.h>

namespace
{
const location::Event::Type id = location::Event::register_type<location::events::WifiAndCellIdReportingStateChanged>("location::events::WifiAndCellIdReportingStateChanged");
}

struct location::events::WifiAndCellIdReportingStateChanged::Private
{
    explicit Private(location::WifiAndCellIdReportingState new_state) : new_state{new_state}
    {
    }

    location::WifiAndCellIdReportingState new_state;
};

location::events::WifiAndCellIdReportingStateChanged::WifiAndCellIdReportingStateChanged(WifiAndCellIdReportingState new_state)
    : d{new Private{new_state}}
{
}

location::events::WifiAndCellIdReportingStateChanged::WifiAndCellIdReportingStateChanged(const WifiAndCellIdReportingStateChanged& rhs)
    : d{new Private{*rhs.d}}
{
}

location::events::WifiAndCellIdReportingStateChanged::WifiAndCellIdReportingStateChanged(WifiAndCellIdReportingStateChanged&& rhs)
    : d{std::move(rhs.d)}
{
}

location::events::WifiAndCellIdReportingStateChanged::~WifiAndCellIdReportingStateChanged() = default;

location::events::WifiAndCellIdReportingStateChanged& location::events::WifiAndCellIdReportingStateChanged::operator=(const WifiAndCellIdReportingStateChanged& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::events::WifiAndCellIdReportingStateChanged& location::events::WifiAndCellIdReportingStateChanged::operator=(WifiAndCellIdReportingStateChanged&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

location::WifiAndCellIdReportingState location::events::WifiAndCellIdReportingStateChanged::new_state() const
{
    return d->new_state;
}

location::Event::Type location::events::WifiAndCellIdReportingStateChanged::type() const
{
    return id;
}

location::Event::Type location::TypeOf<location::events::WifiAndCellIdReportingStateChanged>::query()
{
    return id;
}
