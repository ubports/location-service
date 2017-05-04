// Copyright (C) 2017 Thomas Voss <thomas.voss.bochum@gmail.com>
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

#ifndef LOCATION_PROVIDERS_SIRF_BITS_H_
#define LOCATION_PROVIDERS_SIRF_BITS_H_

#include <cstdint>

namespace location
{
namespace providers
{
namespace sirf
{
namespace bits
{

template<std::size_t begin, std::size_t end, typename T, typename U>
void set(T& bitfield, U value)
{
    static_assert(begin < end, "begin >= end");
    bitfield |= ((value & ((1 << (end - begin))-1)) << begin);
}

template<std::size_t begin, std::size_t end, typename T, typename U = T>
U get(const T& bitfield)
{
    static_assert(begin < end, "begin >= end");
    return (bitfield >> begin) & ((1 << (end - begin)) - 1);
}

}  // namespace bits
}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif  // LOCATION_PROVIDERS_SIRF_BITS_H_
