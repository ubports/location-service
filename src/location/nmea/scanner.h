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
#ifndef LOCATION_NMEA_SCANNER_H_
#define LOCATION_NMEA_SCANNER_H_

#include <sstream>
#include <string>

namespace location
{
namespace nmea
{

/// @brief Scanner inspects incoming characters and tries to identify beginning
/// and end of
/// NMEA messages.
class Scanner
{
public:
    /// @brief Expect models the state of the scanner, describing what it expects
    /// next
    /// to advance its state.
    enum class Expect
    {
        dollar,
        more_data,
        line_feed,
        nothing_more
    };

    /// @brief update updates the state of the Scanner with c.
    Expect update(char c);

    /// @brief finalize tries to extract a complete NMEA sentence.
    ///
    /// Throws a std::runtime_error if the Scanner is not in state
    /// Expect::nothing_more.
    std::string finalize();

private:
    Expect state{Expect::dollar}; ///< The state of the Scanner.
    std::stringstream ss;         ///< Buffer holding incomplete sentence data.
};

}
}

#endif // LOCATION_NMEA_SCANNER_H_
