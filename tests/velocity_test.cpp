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
#include "com/ubuntu/location/velocity.h"

#include <gtest/gtest.h>

TEST(Velocity, constructing_a_velocity_with_invalid_value_throws)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(
        -std::numeric_limits<double>::max(),
        com::ubuntu::location::Velocity::min().value() - std::numeric_limits<double>::min());
    double d = dist(rng);
    EXPECT_ANY_THROW(com::ubuntu::location::Velocity v {d*com::ubuntu::location::units::MetersPerSecond};);
    dist = std::uniform_real_distribution<double>(
        com::ubuntu::location::Velocity::max().value() + std::numeric_limits<double>::min(),
        std::numeric_limits<double>::max());
    d = dist(rng);
    EXPECT_NO_THROW(com::ubuntu::location::Velocity v {d*com::ubuntu::location::units::MetersPerSecond};);
}

TEST(Velocity, constructing_a_velocity_with_a_valid_value_does_not_throw)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Velocity::min().value(),
            com::ubuntu::location::Velocity::max().value());

    EXPECT_NO_THROW(com::ubuntu::location::Velocity v {dist(rng)*com::ubuntu::location::units::MetersPerSecond};);
}

TEST(Velocity, a_velocity_contains_value_passed_at_construction)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Velocity::min().value(),
            com::ubuntu::location::Velocity::max().value());
    double d = dist(rng);
    com::ubuntu::location::Velocity v {d* com::ubuntu::location::units::MetersPerSecond};
    EXPECT_EQ(d * com::ubuntu::location::units::MetersPerSecond, v.value);
}
