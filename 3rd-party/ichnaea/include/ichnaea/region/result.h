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
#ifndef ICHNAEA_REGION_RESULT_H_
#define ICHNAEA_REGION_RESULT_H_

#include <ichnaea/geolocate/fallback.h>

namespace ichnaea
{
namespace region
{
/// @brief Result bundles the result of a region query, reporting back
/// country code and name using region codes and names from the GENC dataset.
///
/// See http://www.gwg.nga.mil/ccwg.php for further details.
struct Result
{
    std::string country_code;
    std::string country_name;
};
}
}

#endif // ICHNAEA_REGION_RESULT_H_
