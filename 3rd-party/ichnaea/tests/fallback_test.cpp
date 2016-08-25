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
#include <ichnaea/geolocate/fallback.h>

#include <gtest/gtest.h>

#include <sstream>

TEST(GeolocateFallback, stream_insertion_operator_produces_correct_result)
{
    {std::stringstream ss; ss << ichnaea::geolocate::Fallback::ip; EXPECT_EQ("ipf", ss.str());}
    {std::stringstream ss; ss << ichnaea::geolocate::Fallback::lac; EXPECT_EQ("lacf", ss.str());}
}

TEST(GeolocateFallback, bitwise_or_works)
{
    EXPECT_TRUE(((ichnaea::geolocate::Fallback::none | ichnaea::geolocate::Fallback::ip)&ichnaea::geolocate::Fallback::ip) == ichnaea::geolocate::Fallback::ip);
    EXPECT_TRUE(((ichnaea::geolocate::Fallback::none | ichnaea::geolocate::Fallback::lac)&ichnaea::geolocate::Fallback::lac) == ichnaea::geolocate::Fallback::lac);
}

TEST(GeolocateFallback, bitwise_and_works)
{
    EXPECT_TRUE((ichnaea::geolocate::Fallback::ip&ichnaea::geolocate::Fallback::ip) == ichnaea::geolocate::Fallback::ip);
    EXPECT_TRUE((ichnaea::geolocate::Fallback::none | ichnaea::geolocate::Fallback::lac&ichnaea::geolocate::Fallback::lac) == ichnaea::geolocate::Fallback::lac);
}

