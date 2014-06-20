/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <com/ubuntu/location/position.h>

#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;

TEST(Position, AllFieldsAreInvalidForDefaultConstructor)
{
    cul::Position p;
    EXPECT_FALSE(p.altitude);
    EXPECT_FALSE(p.accuracy.vertical);
}

TEST(Position, InitWithLatLonGivesValidFieldsForLatLon)
{
    cul::Position p{cul::wgs84::Latitude{}, cul::wgs84::Longitude{}};
    EXPECT_FALSE(p.altitude);
}

TEST(Position, InitWithLatLonAltGivesValidFieldsForLatLonAlt)
{
    cul::Position p{
        cul::wgs84::Latitude{},
        cul::wgs84::Longitude{},
        cul::wgs84::Altitude{}};
    EXPECT_TRUE(p.altitude);
}

#include <com/ubuntu/location/codec.h>

#include <core/dbus/message_streaming_operators.h>

TEST(Position, EncodingAndDecodingGivesSameResults)
{
    cul::Position p
    {
        cul::wgs84::Latitude{9. * cul::units::Degrees},
        cul::wgs84::Longitude{53. * cul::units::Degrees},
        cul::wgs84::Altitude{-2. * cul::units::Meters}
    };

    p.accuracy.horizontal =  cul::Position::Accuracy::Horizontal{300*cul::units::Meters};
    p.accuracy.vertical = cul::Position::Accuracy::Vertical{100*cul::units::Meters};

    auto msg = core::dbus::Message::make_method_call(
        "org.freedesktop.DBus",
        core::dbus::types::ObjectPath("/org/freedesktop/DBus"),
        "org.freedesktop.DBus",
        "ListNames");

    msg->writer() << p;

    cul::Position pp;
    msg->reader() >> pp;

    EXPECT_EQ(p, pp);
}
