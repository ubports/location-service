/*
 * Copyright © 2014 Canonical Ltd.
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
#ifndef MOCK_PROVIDER_H_
#define MOCK_PROVIDER_H_

#include <location/provider.h>

#include <gmock/gmock.h>

struct MockProvider : public location::Provider
{
    MockProvider() : location::Provider()
    {
    }

    MOCK_METHOD1(matches_criteria, bool(const location::Criteria&));

    MOCK_CONST_METHOD1(supports, bool(const location::Provider::Features&));
    MOCK_CONST_METHOD1(requires, bool(const location::Provider::Requirements&));

    // Called by the engine whenever the wifi and cell ID reporting state changes.
    MOCK_METHOD1(on_wifi_and_cell_reporting_state_changed, void(location::WifiAndCellIdReportingState state));

    // Called by the engine whenever the reference location changed.
    MOCK_METHOD1(on_reference_location_updated, void(const location::Update<location::Position>& position));

    // Called by the engine whenever the reference velocity changed.
    MOCK_METHOD1(on_reference_velocity_updated, void(const location::Update<location::Velocity>& velocity));

    // Called by the engine whenever the reference heading changed.
    MOCK_METHOD1(on_reference_heading_updated, void(const location::Update<location::Heading>& heading));

    MOCK_METHOD0(start_position_updates, void());
    MOCK_METHOD0(stop_position_updates, void());

    MOCK_METHOD0(start_heading_updates, void());
    MOCK_METHOD0(stop_heading_updates, void());

    MOCK_METHOD0(start_velocity_updates, void());
    MOCK_METHOD0(stop_velocity_updates, void());

    // Inject a position update from the outside.
    void inject_update(const location::Update<location::Position>& update)
    {
        mutable_updates().position(update);
    }

    // Inject a velocity update from the outside.
    void inject_update(const location::Update<location::Velocity>& update)
    {
        mutable_updates().velocity(update);
    }

    // Inject a heading update from the outside.
    void inject_update(const location::Update<location::Heading>& update)
    {
        mutable_updates().heading(update);
    }
};

#endif // MOCK_PROVIDER_H_
