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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_WIFI_AND_CELL_REPORTING_STATE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_WIFI_AND_CELL_REPORTING_STATE_H_

namespace com
{
namespace ubuntu
{
namespace location
{
enum class WifiAndCellIdReportingState
{
    on, ///< Wifi and Cell Ids might be reported to online location services.
    off ///< Wifi and Cell Ids are _not_ reported. This is the default value.
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_WIFI_AND_CELL_REPORTING_STATE_H_
