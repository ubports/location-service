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
#ifndef ICHNAEA_GEOLOCATE_FALLBACK_H_
#define ICHNAEA_GEOLOCATE_FALLBACK_H_

#include <iosfwd>

namespace ichnaea
{
namespace geolocate
{
/// @brief Fallback enumerates all known fallback strategies
/// for obtaining a position estimate in the case of missing or
/// contradictory measurements.
enum class Fallback
{
    none    = 0,
    /// If no exact cell match can be found, fall back from exact cell
    /// position estimates to more coarse grained cell location area estimates,
    /// rather than going directly to an even worse GeoIP based estimate.
    lac     = 1 << 0,
    /// If no position can be estimated based on any of the provided data points,
    /// fall back to an estimate based on a GeoIP database based on the senders IP
    /// address at the time of the query.
    ip      = 1 << 1
};

/// @brief operator<< inserts fallback into out.
std::ostream& operator<<(std::ostream& out, Fallback fallback);
/// @brief operator| returns the bitwise or of lhs and rhs.
Fallback operator|(Fallback lhs, Fallback rhs);
/// @brief operator| returns the bitwise and of lhs and rhs.
Fallback operator&(Fallback lhs, Fallback rhs);
}
}

#endif // ICHNAEA_GEOLOCATE_FALLBACK_H_
