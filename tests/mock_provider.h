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
    MOCK_METHOD1(on_new_event, void(const location::Event&));
    MOCK_METHOD0(enable, void());
    MOCK_METHOD0(disable, void());
    MOCK_METHOD0(activate, void());
    MOCK_METHOD0(deactivate, void());

    MOCK_CONST_METHOD0(requirements, Requirements());
    MOCK_METHOD1(satisfies, bool(const location::Criteria&));
    MOCK_CONST_METHOD0(position_updates, const core::Signal<location::Update<location::Position>>&());
    MOCK_CONST_METHOD0(heading_updates, const core::Signal<location::Update<location::units::Degrees>>&());
    MOCK_CONST_METHOD0(velocity_updates, const core::Signal<location::Update<location::units::MetersPerSecond>>&());

    // Inject a position update from the outside.
    void inject_update(const location::Update<location::Position>& update)
    {
        updates.position(update);
    }

    // Inject a velocity update from the outside.
    void inject_update(const location::Update<location::units::MetersPerSecond>& update)
    {
        updates.velocity(update);
    }

    // Inject a heading update from the outside.
    void inject_update(const location::Update<location::units::Degrees>& update)
    {
        updates.heading(update);
    }

    struct
    {
        core::Signal<location::Update<location::Position>> position;
        core::Signal<location::Update<location::units::Degrees>> heading;
        core::Signal<location::Update<location::units::MetersPerSecond>> velocity;
    } updates;
};

#endif // MOCK_PROVIDER_H_
