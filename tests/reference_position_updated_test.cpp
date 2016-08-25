/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/events/reference_position_updated.h>

#include <gtest/gtest.h>

namespace
{
location::Position reference_position
{
    location::wgs84::Latitude
    {
        9. * location::units::Degrees
    },
    location::wgs84::Longitude
    {
        53. * location::units::Degrees
    },
    location::wgs84::Altitude
    {
        -2. * location::units::Meters
    }
};

location::Update<location::Position> reference_update{reference_position};
}

TEST(ReferencePositionUpdated, ctor_and_dtor_work)
{
    location::events::ReferencePositionUpdated event{reference_update};
}

TEST(ReferencePositionUpdated, ctor_initiales_state_value)
{
    location::events::ReferencePositionUpdated event{reference_update};
    EXPECT_EQ(reference_update, event.update());
}

TEST(ReferencePositionUpdated, returns_correct_type_from_instance)
{
    location::events::ReferencePositionUpdated event{reference_update};
    EXPECT_EQ(location::TypeOf<location::events::ReferencePositionUpdated>::query(), event.type());
}
