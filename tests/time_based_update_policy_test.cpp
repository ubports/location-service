/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Manuel de la Pena <manuel.delapena@canonical.com>
 */
#include <location/time_based_update_policy.h>

#include <gtest/gtest.h>

using namespace ::testing;
namespace cul = location;

namespace
{
    auto timestamp = location::Clock::now();

    location::Update<location::Position> reference_position_update
            {
                    {
                            location::wgs84::Latitude{9. * location::units::Degrees},
                            location::wgs84::Longitude{53. * location::units::Degrees},
                            location::wgs84::Altitude{-2. * location::units::Meters},
                    },
                    timestamp
            };
}

// make certain internal details public so that we can set the last update
class PublicTimeBasedUpdatePolicy : public cul::TimeBasedUpdatePolicy {
 public:
    PublicTimeBasedUpdatePolicy(std::chrono::minutes mins) :  cul::TimeBasedUpdatePolicy(mins) {}
    using cul::TimeBasedUpdatePolicy::last_position_update;
    using cul::TimeBasedUpdatePolicy::last_heading_update;
    using cul::TimeBasedUpdatePolicy::last_velocity_update;
};

TEST(TimeBasedUpdatePolicy, policy_ignores_updates_that_are_too_old)
{
    auto policy = std::make_shared<PublicTimeBasedUpdatePolicy>(std::chrono::minutes(2));
    policy->last_position_update = reference_position_update;

    location::Update<location::Position> old_update
            {
                    {
                            location::wgs84::Latitude{10. * location::units::Degrees},
                            location::wgs84::Longitude{60. * location::units::Degrees},
                            location::wgs84::Altitude{10. * location::units::Meters}
                    },
                    timestamp - std::chrono::minutes(5)
            };
    policy->verify_update(old_update);

    ASSERT_NE(policy->last_position_update.value.latitude, old_update.value.latitude);
    ASSERT_EQ(policy->last_position_update.value.latitude, reference_position_update.value.latitude);

    ASSERT_NE(policy->last_position_update.value.longitude, old_update.value.longitude);
    ASSERT_EQ(policy->last_position_update.value.longitude, reference_position_update.value.longitude);

    ASSERT_NE(policy->last_position_update.value.altitude, old_update.value.altitude);
    ASSERT_EQ(policy->last_position_update.value.altitude, reference_position_update.value.altitude);
}

TEST(TimeBasedUpdatePolicy, policy_uses_very_recent_updates)
{
    auto policy = std::make_shared<PublicTimeBasedUpdatePolicy>(std::chrono::minutes(2));

    policy->last_position_update = reference_position_update;

    location::Update<location::Position> new_update
            {
                    {
                            location::wgs84::Latitude{10. * location::units::Degrees},
                            location::wgs84::Longitude{60. * location::units::Degrees},
                            location::wgs84::Altitude{10. * location::units::Meters}
                    },
                    timestamp + std::chrono::minutes(3)
            };

    policy->verify_update(new_update);

    ASSERT_EQ(policy->last_position_update.value.latitude, new_update.value.latitude);
    ASSERT_NE(policy->last_position_update.value.latitude, reference_position_update.value.latitude);

    ASSERT_EQ(policy->last_position_update.value.longitude, new_update.value.longitude);
    ASSERT_NE(policy->last_position_update.value.longitude, reference_position_update.value.longitude);

    ASSERT_EQ(policy->last_position_update.value.altitude, new_update.value.altitude);
    ASSERT_NE(policy->last_position_update.value.altitude, reference_position_update.value.altitude);
}

TEST(TimeBasedUpdatePolicy, policy_ignores_inaccurate_updates)
{
    auto policy = std::make_shared<PublicTimeBasedUpdatePolicy>(std::chrono::minutes(2));
    reference_position_update.value.accuracy.horizontal = 1. * location::units::Meters;
    policy->last_position_update = reference_position_update;

    location::Update<location::Position> new_update
            {
                    {
                            location::wgs84::Latitude{10. * location::units::Degrees},
                            location::wgs84::Longitude{60. * location::units::Degrees},
                            location::wgs84::Altitude{10. * location::units::Meters},
                    },
                    timestamp + std::chrono::minutes(1)
            };
    new_update.value.accuracy.horizontal = 8. * location::units::Meters;

    policy->verify_update(new_update);
    ASSERT_TRUE(*new_update.value.accuracy.horizontal > *reference_position_update.value.accuracy.horizontal);

    ASSERT_NE(policy->last_position_update.value.latitude, new_update.value.latitude);
    ASSERT_EQ(policy->last_position_update.value.latitude, reference_position_update.value.latitude);

    ASSERT_NE(policy->last_position_update.value.longitude, new_update.value.longitude);
    ASSERT_EQ(policy->last_position_update.value.longitude, reference_position_update.value.longitude);

    ASSERT_NE(policy->last_position_update.value.altitude, new_update.value.altitude);
    ASSERT_EQ(policy->last_position_update.value.altitude, reference_position_update.value.altitude);
}
