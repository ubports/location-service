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
#include <com/ubuntu/location/criteria.h>

#include <gtest/gtest.h>

namespace location = com::ubuntu::location;

TEST(Criteria, SatisfiesReturnsFalseForNonSatisfyingCriteria)
{
    location::Criteria c1, c2;
    c2.requires.altitude = true;

    EXPECT_FALSE(c1.satisfies(c2));
}

TEST(Criteria, AddedUpCriteriaSatisfiesAllIndividualCriteria)
{
    location::Criteria c1, c2, c3;

    c2.requires.altitude = true;
    c2.requires.velocity = true;

    c3.accuracy.horizontal = 20 * location::units::Meters;

    // We make sure that the trivial case is not true.
    EXPECT_FALSE(c1.satisfies(c2));
    EXPECT_FALSE(c1.satisfies(c3));

    auto added_up_criteria = c1 + c2 + c3;

    EXPECT_TRUE(added_up_criteria.satisfies(c1));
    EXPECT_TRUE(added_up_criteria.satisfies(c2));
    EXPECT_TRUE(added_up_criteria.satisfies(c3));
}
