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
#ifndef LOCATION_BUS_H_
#define LOCATION_BUS_H_

#include <location/event.h>

#include <memory>

namespace location
{
/// @brief Bus provides a simple way for distributing events to multiple listeners.
class Bus
{
public:
    // Safe us some typing
    typedef std::shared_ptr<Bus> Ptr;

    /// @brief subscribe makes receiver known to the bus.
    virtual void subscribe(const Event::Receiver::Ptr& receiver) = 0;

    /// @brief unsubscribe removes receiver from the bus.
    virtual void unsubscribe(const Event::Receiver::Ptr& receiver) = 0;

    /// @brief dispatch takes eventand hands it to all subscribed receivers.
    virtual void dispatch(const Event::Ptr& event) = 0;
};
}

#endif // LOCATION_BUS_H_
