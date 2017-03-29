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

#include <location/providers/ubx/_8/nmea/scanner.h>

#include <stdexcept>

namespace nmea = location::providers::ubx::_8::nmea;

nmea::Scanner::Expect nmea::Scanner::update(char c) {
  switch (state) {
    case Expect::dollar:
      if (c == '$') {
        ss << c;
        state = Expect::more_data;
      }
      break;
    case Expect::more_data:
      switch (c) {
        case '\r':
          state = Expect::line_feed;
          break;
        default:
          break;
      }
      ss << c;
      break;
    case Expect::line_feed:
      switch (c) {
        case '\n':
          state = Expect::nothing_more;
          break;
        default:
          break;
      }
      ss << c;
      break;
    default:
      break;
  }

  return state;
}

std::string nmea::Scanner::finalize() {
  if (state != Expect::nothing_more) throw std::runtime_error{"Incomplete"};

  auto result = ss.str();
  ss.str("");
  ss.clear();
  state = Expect::dollar;

  return result;
}
