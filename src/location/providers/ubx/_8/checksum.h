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

#ifndef UBX_8_CHECKSUM_H_
#define UBX_8_CHECKSUM_H_

#include <cstdint>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class Checksum
{
public:
    void operator()(std::uint8_t byte);

    std::uint8_t ck_a() const;
    std::uint8_t ck_b() const;

private:
    std::uint8_t ck_a_{0};
    std::uint8_t ck_b_{0};
};

}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namepsace location

#endif  // UBX_8_CHECKSUM_H_
