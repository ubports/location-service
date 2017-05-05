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

#ifndef LOCATION_PROVIDERS_SIRF_MAGIC_H_
#define LOCATION_PROVIDERS_SIRF_MAGIC_H_

namespace location
{
namespace providers
{
namespace sirf
{
namespace begin
{

constexpr const std::uint8_t sync_char_1{0xa0};
constexpr const std::uint8_t sync_char_2{0xa2};

}  // namespace begin
namespace end
{

constexpr const std::uint8_t sync_char_1{0xb0};
constexpr const std::uint8_t sync_char_2{0xb3};

}  // namespace end
}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif // LOCATION_PROVIDERS_SIRF_MAGIC_H_
