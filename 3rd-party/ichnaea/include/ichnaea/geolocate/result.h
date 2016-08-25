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
#ifndef ICHNAEA_GEOLOCATE_RESULT_H_
#define ICHNAEA_GEOLOCATE_RESULT_H_

#include <ichnaea/geolocate/fallback.h>

#include <boost/optional.hpp>

namespace ichnaea
{
namespace geolocate
{
/// @brief Result bundles the successful respsone to a
/// geolocate request.
///
/// Please note that we provide accessors for convenience and to
/// implement a named parameter pattern.
struct Result
{
    struct Location
    {
        double lat; ///< lat is the latitude component of the position estimate.
        double lon; ///< lon is the longitude component of the position estimate.
    };
    Location location;                  ///< location is the position estimate determined by the service.
    double   accuracy;                  ///< accuracy describes the quality of the estimate in terms of a circle with radious accuracy.
    boost::optional<Fallback> fallback; ///< fallback contains the fallback strategies that were used to obtain the position estimate.
};
}
}

#endif // ICHNAEA_GEOLOCATE_RESULT_H_
