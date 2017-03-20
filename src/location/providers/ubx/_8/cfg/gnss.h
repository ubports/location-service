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

#ifndef UBX_8_CFG_GNSS_H_
#define UBX_8_CFG_GNSS_H_

#include <location/optional.h>

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

struct Gnss
{
    template<std::uint8_t ID>
    struct Constellation
    {
        static constexpr const std::uint8_t id{ID};

        bool enable;
        std::uint8_t min_tracking_channels = 0;
        std::uint8_t max_tracking_channels = 0;
    };

    struct BeiDou : public Constellation<3>
    {
        bool b1l = false;
    };

    struct Glonass : public Constellation<6>
    {
        bool l1of = false;
    };

    struct Gps : public Constellation<0>
    {
        bool l1ca = false;
    };

    struct Galileo : public Constellation<2>
    {
        bool e1os = false;
    };

    struct Imes : public Constellation<4>
    {
        bool l1 = false;
    };

    struct Qzss : public Constellation<5>
    {
        bool l1ca = false;
        bool l1saif = false;
    };

    struct Sbas : public Constellation<1>
    {
        bool l1ca = false;
    };

    static constexpr std::uint8_t class_id{0x06};
    static constexpr std::uint8_t message_id{0x3e};

    std::size_t size() const;
    bool write(Writer& writer) const;

    std::uint8_t version = 0;
    std::uint8_t tracking_channels_hw = 0;
    std::uint8_t tracking_channels_used = 0xff;

    Optional<BeiDou> beidou;
    Optional<Glonass> glonass;
    Optional<Gps> gps;
    Optional<Galileo> galileo;
    Optional<Imes> imes;
    Optional<Qzss> qzss;
    Optional<Sbas> sbas;
};

std::ostream& operator<<(std::ostream& out, const Gnss& gnss);

}  // namespace cfg
}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif  // UBX_8_CFG_GNSS_H_
