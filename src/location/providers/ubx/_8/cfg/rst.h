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

#ifndef UBX_8_CFG_RST_H_
#define UBX_8_CFG_RST_H_

#include <cstdint>
#include <iosfwd>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class Writer;

namespace cfg
{

struct Rst
{
    static constexpr std::uint8_t class_id{0x06};
    static constexpr std::uint8_t message_id{0x04};

    enum Bits
    {
        eph = 1 << 0,
        alm = 1 << 1,
        health = 1 << 2,
        klob = 1 << 3,
        pos = 1 << 4,
        clkd = 1 << 5,
        osc = 1 << 6,
        utc = 1 << 7,
        rtc = 1 << 8,
        aop = 1 << 15,

        hot_start = 0,
        warm_start = eph,
        cold_start = eph | alm | health | klob | pos | clkd | osc | utc | rtc | aop
    };

    enum Mode
    {
        hardware_reset = 0x00,
        controlled_software_reset = 0x01,
        controlled_software_reset_gnss = 0x02,
        hardware_reset_after_shutdown = 0x04,
        controlled_gnss_stop = 0x08,
        controlled_gnss_start = 0x09
    };

    std::size_t size() const;
    void write(Writer& writer) const;

    Bits bits;
    Mode mode;
};

std::ostream& operator<<(std::ostream& out, const Rst& rst);

}  // namespace cfg
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_CFG_RST_H_
