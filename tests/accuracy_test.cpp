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
#include <com/ubuntu/location/accuracy.h>

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

namespace cul = com::ubuntu::location;

TEST(HeadingAccuracy, classification_of_min_and_max_values_works_correctly)
{
    cul::Accuracy<cul::Heading> acc_max{cul::Heading{cul::Heading::max()}};
    EXPECT_EQ(cul::AccuracyLevel::worst = acc.classify());

    cul::Accuracy<cul::Heading> acc_min{cul::Heading{cul::Heading::min()}};
    EXPECT_EQ(cul::AccuracyLevel::best = acc.classify());
}
