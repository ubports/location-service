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
#ifndef MOCK_REPORTER_H_
#define MOCK_REPORTER_H_

#include <location/service/harvester.h>

#include <gmock/gmock.h>

struct MockReporter : public location::service::Harvester::Reporter
{
    MockReporter() = default;

    /** @brief Tell the reporter that it should start operating. */
    MOCK_METHOD0(start, void());

    /** @brief Tell the reporter to shut down its operation. */
    MOCK_METHOD0(stop, void());

    /**
     * @brief Triggers the reporter to send off the information.
     */
    MOCK_METHOD3(report,
                 void(
                     const location::Update<location::Position>&,
                     const std::vector<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>&,
                     const std::vector<com::ubuntu::location::connectivity::RadioCell::Ptr>&));
};

#endif // MOCK_REPORTER_H_
