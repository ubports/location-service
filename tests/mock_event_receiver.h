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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *              Scott Sweeny <scott.sweeny@canonical.com>
 */
#ifndef MOCK_EVENT_RECEIVER_H_
#define MOCK_EVENT_RECEIVER_H_

#include <location/update.h>

#include <gmock/gmock.h>

struct MockEventReceiver
{
    MOCK_METHOD1(position_update_received, void(const location::Update<location::Position>&));
    MOCK_METHOD1(heading_update_received, void(const location::Update<location::Heading>&));
    MOCK_METHOD1(velocity_update_received, void(const location::Update<location::Velocity>&));
};

#endif // MOCK_EVENT_RECEIVER_H_
