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

#ifndef LOCATION_PROVIDERS_SIRF_READER_H_
#define LOCATION_PROVIDERS_SIRF_READER_H_

#include <cstdint>
#include <vector>

namespace location
{
namespace providers
{
namespace sirf
{

class Reader
{
public:
    explicit Reader(std::vector<std::uint8_t>::const_iterator begin,
                    std::vector<std::uint8_t>::const_iterator end);

    std::uint8_t read_unsigned_char();
    std::int8_t read_signed_char();
    std::uint16_t read_unsigned_short();
    std::int16_t read_signed_short();
    std::uint32_t read_unsigned_long();
    std::int32_t read_signed_long();

private:
    std::vector<std::uint8_t>::const_iterator begin;
    std::vector<std::uint8_t>::const_iterator current;
    std::vector<std::uint8_t>::const_iterator end;
};

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif // LOCATION_PROVIDERS_SIRF_READER_H_
