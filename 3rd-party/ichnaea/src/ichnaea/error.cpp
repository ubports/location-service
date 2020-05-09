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
#include <ichnaea/error.h>

#include <iostream>

ichnaea::Error::Error(core::net::http::Status code, const std::string& message)
    : std::runtime_error{message}, code{code}, message{message}
{
}

std::ostream& ichnaea::operator<<(std::ostream& out, const Error& error)
{
    out << error.code << ", " << error.message << std::endl;
    for (const Error::Detail& detail : error.errors)
        out << "  " << detail.domain << ", " << detail.message << ", " << detail.reason << std::endl;
    return out;
}
