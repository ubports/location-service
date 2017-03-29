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

#include <location/providers/ubx/_8/cfg/gnss.h>

#include <location/providers/ubx/bits.h>
#include <location/providers/ubx/_8/writer.h>

#include <cstdint>
#include <iostream>

namespace cfg = location::providers::ubx::_8::cfg;

std::size_t cfg::Gnss::size() const
{
    std::size_t result = 4;

    if (beidou)
        result += 8;
    if (glonass)
        result += 8;
    if (gps)
        result += 8;
    if (galileo)
        result += 8;
    if (imes)
        result += 8;
    if (qzss)
        result += 8;
    if (sbas)
        result += 8;

    return result;
}

bool cfg::Gnss::write(Writer& writer) const
{
    writer.write_unsigned_char(version);
    writer.write_unsigned_char(tracking_channels_hw);
    writer.write_unsigned_char(tracking_channels_used);

    std::uint8_t num_configs = 0;
    if (beidou)     num_configs++;
    if (glonass)    num_configs++;
    if (gps)        num_configs++;
    if (galileo)    num_configs++;
    if (imes)       num_configs++;
    if (qzss)       num_configs++;
    if (sbas)       num_configs++;

    writer.write_unsigned_char(num_configs);

    if (beidou)
    {
        writer.write_unsigned_char(BeiDou::id);
        writer.write_unsigned_char(beidou.get().min_tracking_channels);
        writer.write_unsigned_char(beidou.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, beidou.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, beidou.get().b1l ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (glonass)
    {
        writer.write_unsigned_char(Glonass::id);
        writer.write_unsigned_char(glonass.get().min_tracking_channels);
        writer.write_unsigned_char(glonass.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, glonass.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, glonass.get().l1of ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (gps)
    {
        writer.write_unsigned_char(Gps::id);
        writer.write_unsigned_char(gps.get().min_tracking_channels);
        writer.write_unsigned_char(gps.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, gps.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, gps.get().l1ca ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (galileo)
    {
        writer.write_unsigned_char(Galileo::id);
        writer.write_unsigned_char(galileo.get().min_tracking_channels);
        writer.write_unsigned_char(galileo.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, galileo.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, galileo.get().e1os ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (imes)
    {
        writer.write_unsigned_char(Imes::id);
        writer.write_unsigned_char(imes.get().min_tracking_channels);
        writer.write_unsigned_char(imes.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, imes.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, imes.get().l1 ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (imes)
    {
        writer.write_unsigned_char(Imes::id);
        writer.write_unsigned_char(imes.get().min_tracking_channels);
        writer.write_unsigned_char(imes.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, imes.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, imes.get().l1 ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (qzss)
    {
        writer.write_unsigned_char(Qzss::id);
        writer.write_unsigned_char(qzss.get().min_tracking_channels);
        writer.write_unsigned_char(qzss.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, qzss.get().enable ? 1 : 0);
        // TODO(tvoss): Consider l1saif here, too.
        bits::set<16, 23>(flags, qzss.get().l1ca ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    if (sbas)
    {
        writer.write_unsigned_char(Sbas::id);
        writer.write_unsigned_char(sbas.get().min_tracking_channels);
        writer.write_unsigned_char(sbas.get().max_tracking_channels);
        writer.write_unsigned_char(0);

        std::uint32_t flags{0};
        bits::set<0, 1>(flags, sbas.get().enable ? 1 : 0);
        bits::set<16, 23>(flags, sbas.get().l1ca ? 0x01: 0x0);

        writer.write_unsigned_long(flags);
    }

    return true;
}

std::ostream& cfg::operator<<(std::ostream& out, const cfg::Gnss& gnss)
{
    out << "cfg-gnss:" << std::endl
        << "  version: " << std::uint32_t(gnss.version) << std::endl
        << "  tracking_channels_hw: " << std::uint32_t(gnss.tracking_channels_hw) << std::endl
        << "  tracking_channels_used: " << std::uint32_t(gnss.tracking_channels_used);

    if (gnss.beidou)
    {
        out << "  beidou:" << std::endl
            << "    enable: " << std::boolalpha << gnss.beidou.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.beidou.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.beidou.get().max_tracking_channels << std::endl
            << "    b1l: " << gnss.beidou.get().b1l;
    }

    if (gnss.glonass)
    {
        out << "  glonass:" << std::endl
            << "    enable: " << std::boolalpha << gnss.glonass.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.glonass.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.glonass.get().max_tracking_channels << std::endl
            << "    l1of: " << gnss.glonass.get().l1of;
    }

    if (gnss.gps)
    {
        out << "  gps:" << std::endl
            << "    enable: " << std::boolalpha << gnss.gps.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.gps.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.gps.get().max_tracking_channels << std::endl
            << "    l1ca: " << gnss.gps.get().l1ca;
    }

    if (gnss.galileo)
    {
        out << "  galileo:" << std::endl
            << "    enable: " << std::boolalpha << gnss.galileo.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.galileo.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.galileo.get().max_tracking_channels << std::endl
            << "    e1os: " << gnss.galileo.get().e1os;
    }

    if (gnss.imes)
    {
        out << "  imes:" << std::endl
            << "    enable: " << std::boolalpha << gnss.imes.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.imes.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.imes.get().max_tracking_channels << std::endl
            << "    l1: " << gnss.imes.get().l1;
    }

    if (gnss.qzss)
    {
        out << "  qzss:" << std::endl
            << "    enable: " << std::boolalpha << gnss.qzss.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.qzss.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.qzss.get().max_tracking_channels << std::endl
            << "    l1ca: " << gnss.qzss.get().l1ca << std::endl
            << "    l1saif: " << gnss.qzss.get().l1saif;
    }

    if (gnss.sbas)
    {
        out << "  sbas:" << std::endl
            << "    enable: " << std::boolalpha << gnss.sbas.get().enable << std::endl
            << "    min_tracking_channels: " << gnss.sbas.get().min_tracking_channels << std::endl
            << "    max_tracking_channels: " << gnss.sbas.get().max_tracking_channels << std::endl
            << "    l1ca: " << gnss.sbas.get().l1ca;
    }

    return out;
}
