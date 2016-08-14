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
#include <ichnaea/response.h>

#include <gtest/gtest.h>

TEST(Response, yields_error_instance_if_initialized_with_error)
{
    ichnaea::Error e{core::net::http::Status::ok, ""};
    ichnaea::Response<double> r{e};
    EXPECT_TRUE(r.is_error());
    EXPECT_NO_THROW(r.error());
    EXPECT_ANY_THROW(r.result());
}

TEST(Response, yields_result_instance_if_initialized_with_result)
{
    ichnaea::Response<unsigned int> r{42};
    EXPECT_FALSE(r.is_error());
    EXPECT_EQ(42, r.result());
    EXPECT_ANY_THROW(r.error());
}
