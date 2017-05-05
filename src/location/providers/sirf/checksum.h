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

#ifndef LOCATION_PROVIDERS_SIRF_CHECKSUM_H_
#define LOCATION_PROVIDERS_SIRF_CHECKSUM_H_

#include <cstdint>

namespace location
{
namespace providers
{
namespace sirf
{

class Checksum
{
public:
    void operator()(std::uint8_t byte);
    std::uint16_t operator()() const;

private:
    std::uint16_t checksum{0};
};

}  // namespace sirf
}  // namespace providers
}  // namepsace location

#endif  // LOCATION_PROVIDERS_SIRF_CHECKSUM_H_
