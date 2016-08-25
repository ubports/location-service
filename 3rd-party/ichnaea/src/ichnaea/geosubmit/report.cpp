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
#include <ichnaea/geosubmit/report.h>

#include <iostream>

std::ostream& ichnaea::geosubmit::operator<<(std::ostream& out, Report::Position::Source source)
{
    switch (source)
    {
    case Report::Position::Source::fusion: return out << "fusion";
    case Report::Position::Source::gps: return out << "gps";
    case Report::Position::Source::manual: return out << "manual";
    }

    return out;
}
