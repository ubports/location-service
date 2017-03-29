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

#include <location/providers/ubx/_8/checksum.h>

namespace _8 = location::providers::ubx::_8;

void _8::Checksum::operator()(std::uint8_t byte)
{
    ck_a_ += byte;
    ck_b_ += ck_a_;
}

std::uint8_t _8::Checksum::ck_a() const
{
    return ck_a_;
}

std::uint8_t _8::Checksum::ck_b() const
{
    return ck_b_;
}
