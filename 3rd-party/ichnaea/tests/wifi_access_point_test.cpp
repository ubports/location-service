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
#include <ichnaea/wifi_access_point.h>

#include <gtest/gtest.h>

TEST(WifiAccessPoint, stream_insertion_operator_returns_correct_type)
{
    {std::stringstream ss; ss << ichnaea::WifiAccessPoint::Type::_802_11_a; EXPECT_EQ("802.11a", ss.str());}
    {std::stringstream ss; ss << ichnaea::WifiAccessPoint::Type::_802_11_b; EXPECT_EQ("802.11b", ss.str());}
    {std::stringstream ss; ss << ichnaea::WifiAccessPoint::Type::_802_11_g; EXPECT_EQ("802.11g", ss.str());}
    {std::stringstream ss; ss << ichnaea::WifiAccessPoint::Type::_802_11_n; EXPECT_EQ("802.11n", ss.str());}
    {std::stringstream ss; ss << ichnaea::WifiAccessPoint::Type::_802_11_ac; EXPECT_EQ("802.11ac", ss.str());}
}
