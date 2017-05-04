// Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#ifndef LOCATION_PROVIDERS_SIRF_MESSAGE_H_
#define LOCATION_PROVIDERS_SIRF_MESSAGE_H_

#include <location/providers/sirf/geodetic_navigation_data.h>
#include <location/providers/sirf/initialize_data_source.h>
#include <location/providers/sirf/set_protocol.h>

#include <boost/variant.hpp>

#include <iostream>

namespace location
{
namespace providers
{
namespace sirf
{

struct Null {};

using Message = boost::variant<
    Null,
    InitializeDataSource,
    GeodeticNavigationData,
    SetProtocol
>;

inline std::ostream& operator<<(std::ostream& out, const Null&)
{
    return out << "Uninterpreted sirf message";
}


}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif // LOCATION_PROVIDERS_SIRF_MESSAGE_H_
