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
#include <ichnaea/radio_cell.h>

#include <iostream>

bool ichnaea::operator<(const RadioCell& lhs, const RadioCell& rhs)
{
    return std::tie(lhs.radio_type, lhs.serving, lhs.mcc, lhs.mnc, lhs.lac, lhs.id, lhs.age, lhs.psc, lhs.timing_advance, lhs.signal_strength) <
           std::tie(rhs.radio_type, rhs.serving, rhs.mcc, rhs.mnc, rhs.lac, rhs.id, rhs.age, lhs.psc, lhs.timing_advance, lhs.signal_strength);
}

std::ostream& ichnaea::operator<<(std::ostream& out, ichnaea::RadioCell::RadioType radio_type)
{
    switch (radio_type)
    {
    case ichnaea::RadioCell::RadioType::gsm: return out << "gsm";
    case ichnaea::RadioCell::RadioType::lte: return out << "lte";
    case ichnaea::RadioCell::RadioType::wcdma: return out << "wcdma";
    }

    return out;
}
