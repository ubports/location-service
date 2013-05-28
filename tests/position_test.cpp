#include "com/ubuntu/location/position.h"

#include <gtest/gtest.h>

TEST(Position, AllFieldsAreInvalidForDefaultConstructor)
{
    com::ubuntu::location::Position p;
    EXPECT_FALSE(p.has_latitude());
    EXPECT_FALSE(p.has_longitude());
    EXPECT_FALSE(p.has_altitude());
    EXPECT_EQ(0, p.flags().to_ulong());
}

TEST(Position, InitWithLatLonGivesValidFieldsForLatLon)
{
    com::ubuntu::location::Position p{com::ubuntu::location::wgs84::Latitude{}, com::ubuntu::location::wgs84::Longitude{}};
    EXPECT_TRUE(p.has_latitude());
    EXPECT_TRUE(p.has_longitude());
    EXPECT_FALSE(p.has_altitude());
    EXPECT_EQ(3, p.flags().to_ulong());
}

TEST(Position, InitWithLatLonAltGivesValidFieldsForLatLonAlt)
{
    com::ubuntu::location::Position p{
        com::ubuntu::location::wgs84::Latitude{}, 
        com::ubuntu::location::wgs84::Longitude{},
        com::ubuntu::location::wgs84::Altitude{}};
    EXPECT_TRUE(p.has_latitude());
    EXPECT_TRUE(p.has_longitude());
    EXPECT_TRUE(p.has_altitude());
    EXPECT_EQ(7, p.flags().to_ulong());
}

TEST(Position, MutatorsAdjustFieldFlags)
{
    com::ubuntu::location::Position p;
    EXPECT_FALSE(p.has_latitude());
    EXPECT_FALSE(p.has_longitude());
    EXPECT_FALSE(p.has_altitude());
    p.latitude(com::ubuntu::location::wgs84::Latitude{});
    EXPECT_TRUE(p.has_latitude());
    EXPECT_FALSE(p.has_longitude());
    EXPECT_FALSE(p.has_altitude());
    p.longitude(com::ubuntu::location::wgs84::Longitude{});
    EXPECT_TRUE(p.has_latitude());
    EXPECT_TRUE(p.has_longitude());
    EXPECT_FALSE(p.has_altitude());
    p.altitude(com::ubuntu::location::wgs84::Altitude{});
    EXPECT_TRUE(p.has_latitude());
    EXPECT_TRUE(p.has_longitude());
    EXPECT_TRUE(p.has_altitude());
}

#include "com/ubuntu/location/codec.h"

#include "org/freedesktop/dbus/message.h"

TEST(Position, EncodingAndDecodingGivesSameResults)
{

    auto msg = org::freedesktop::dbus::Message::make_method_call(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "ListNames");

    {
        com::ubuntu::location::Position p{
            com::ubuntu::location::wgs84::Latitude{9. * com::ubuntu::location::units::Degrees},
            com::ubuntu::location::wgs84::Longitude{53. * com::ubuntu::location::units::Degrees},
            com::ubuntu::location::wgs84::Altitude{-2. * com::ubuntu::location::units::Meters}};

        msg->writer() << p;
    }

    {
        com::ubuntu::location::Position p; msg->reader() >> p;
        com::ubuntu::location::Position p_ref{
            com::ubuntu::location::wgs84::Latitude{9. * com::ubuntu::location::units::Degrees},
            com::ubuntu::location::wgs84::Longitude{53. * com::ubuntu::location::units::Degrees},
            com::ubuntu::location::wgs84::Altitude{-2. * com::ubuntu::location::units::Meters}};
        EXPECT_EQ(p_ref, p);
    }
}
