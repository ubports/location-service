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

#include <location/service/demultiplexing_reporter.h>

#include <gtest/gtest.h>

#include "mock_reporter.h"

namespace
{
location::Update<location::Position> reference_position_update
{
    {
        location::wgs84::Latitude{9. * location::units::Degrees},
        location::wgs84::Longitude{53. * location::units::Degrees},
        location::wgs84::Altitude{-2. * location::units::Meters}
    },
    location::Clock::now()
};
}
TEST(DemultiplexingReporter, dispatches_calls_to_all_reporters)
{
    using namespace ::testing;

    std::set<location::service::Harvester::Reporter::Ptr> reporters;

    for (unsigned int i = 0; i < 5; i++)
    {
        auto reporter = std::make_shared<MockReporter>();

        EXPECT_CALL(*reporter, start()).Times(1);
        EXPECT_CALL(*reporter, stop()).Times(1);
        EXPECT_CALL(*reporter, report(_, _, _)).Times(1);

        reporters.insert(reporter);
    }

    location::service::DemultiplexingReporter reporter{reporters};

    reporter.start();
    reporter.report(reference_position_update,{}, {});
    reporter.stop();
}
