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
#include <ichnaea/radio_cell.h>

#include <gtest/gtest.h>

#include <sstream>

TEST(RadioCell, stream_insertion_operator_for_type_works)
{
    {std::stringstream ss; ss << ichnaea::RadioCell::RadioType::gsm; EXPECT_EQ("gsm", ss.str());}
    {std::stringstream ss; ss << ichnaea::RadioCell::RadioType::lte; EXPECT_EQ("lte", ss.str());}
    {std::stringstream ss; ss << ichnaea::RadioCell::RadioType::wcdma; EXPECT_EQ("wcdma", ss.str());}
}
