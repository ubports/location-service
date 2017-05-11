// Copyright (C) 2017 Thomas Voss <thomas.voss@canonical.com>
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

#ifndef LOCATION_PROVIDERS_SIRF_SET_MESSAGE_RATE_H_
#define LOCATION_PROVIDERS_SIRF_SET_MESSAGE_RATE_H_

#include <location/providers/sirf/writer.h>

#include <cstdint>
#include <iostream>

namespace location
{
namespace providers
{
namespace sirf
{

struct SetMessageRate
{
    enum Mode
    {
        one_message = 0
    };

    static constexpr std::uint8_t id{166};

    std::size_t size() const
    {
        return 8;
    }

    void write(Writer& writer) const
    {
        writer.write_unsigned_char(id);
        writer.write_unsigned_char(mode);
        writer.write_unsigned_char(message_id);
        writer.write_unsigned_char(cycle_period);
        writer.write_unsigned_char(0);
        writer.write_unsigned_char(0);
        writer.write_unsigned_char(0);
        writer.write_unsigned_char(0);
    }

    Mode mode;
    std::uint8_t message_id;
    std::uint8_t cycle_period;
};

inline std::ostream& operator<<(std::ostream& out, const SetMessageRate& smr)
{
    return out << "SetMessageRate:" << std::endl
               << "  mode: " << smr.mode << std::endl
               << "  message-id: " << smr.message_id << std::endl
               << "  cycle-period: " << smr.cycle_period;
}

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif  // LOCATION_PROVIDERS_SIRF_SET_MESSAGE_RATE_H_
