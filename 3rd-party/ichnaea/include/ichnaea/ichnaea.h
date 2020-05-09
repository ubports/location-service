// Copyright (C) 2016 Canonical Ltd.
// 
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ICHNAEA_ICHNAEA_H_
#define ICHNAEA_ICHNAEA_H_

#include "ichnaea/response.h"
#include "ichnaea/radio_cell.h"
#include "ichnaea/client.h"
#include "ichnaea/error.h"
#include "ichnaea/geosubmit/parameters.h"
#include "ichnaea/geosubmit/result.h"
#include "ichnaea/geosubmit/report.h"
#include "ichnaea/bluetooth_beacon.h"
#include "ichnaea/geolocate/fallback.h"
#include "ichnaea/geolocate/parameters.h"
#include "ichnaea/geolocate/result.h"
#include "ichnaea/wifi_access_point.h"

/// @brief ichnaea contains types and functions for using Mozilla's location services:
///
/// Please see https://mozilla.github.io/ichnaea/index.html for further details on the server-side
/// implementation.
namespace ichnaea{}

#endif // ICHNAEA_ICHNAEA_H_
