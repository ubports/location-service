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
#ifndef UBX_DOP_H_
#define UBX_DOP_H_

namespace location
{
namespace providers
{
namespace ubx
{
namespace tag
{
struct Horizontal
{
};
struct Positional
{
};
struct Vertical
{
};
}
template <typename T>
struct DillusionOfPrecision
{
    explicit DillusionOfPrecision(float value = -1) : value{value} {}

    DillusionOfPrecision(const DillusionOfPrecision<T>& rhs) : value{rhs.value} {}

    DillusionOfPrecision& operator=(const DillusionOfPrecision<T>& rhs)
    {
        value = rhs.value;
        return *this;
    }

    operator float() const { return value; }

    float value;
};

template <typename T>
using Dop = DillusionOfPrecision<T>;
}
}
}

#endif // UBX_DOP_H_
