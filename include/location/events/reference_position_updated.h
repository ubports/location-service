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

#ifndef LOCATION_EVENTS_REFERENCE_POSITION_UPDATED_H_
#define LOCATION_EVENTS_REFERENCE_POSITION_UPDATED_H_

#include <location/event.h>

#include <location/position.h>
#include <location/update.h>

namespace location
{
namespace events
{
/// @brief ReferencePositionUpdated is sent if the reference position has been updated.
class ReferencePositionUpdated : public Event
{
public:
    /// @brief ReferencePositionUpdated initializes a new instance with update.
    ReferencePositionUpdated(const Update<Position>& update);
    /// @brief ReferencePositionUpdated initializes a new instance from rhs.
    ReferencePositionUpdated(const ReferencePositionUpdated& rhs);
    /// @brief operator= assigns rhs to this instance.
    ReferencePositionUpdated& operator=(const ReferencePositionUpdated& rhs);

    /// @brief update returns the contained update.
    const Update<Position>& update() const;

    // From Bus::Message
    Type type() const override;

private:
    Update<Position> update_;
};
}
template<>
struct TypeOf<events::ReferencePositionUpdated>
{
    static Event::Type query();
};
}

#endif // LOCATION_EVENTS_REFERENCE_POSITION_UPDATED_H_
