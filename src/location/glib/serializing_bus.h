/*
 * Copyright © 2017 Canonical Ltd.
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
#ifndef LOCATION_GLIB_SERIALIZING_BUS_H_
#define LOCATION_GLIB_SERIALIZING_BUS_H_

#include <location/bus.h>
#include <location/visibility.h>

#include <unordered_set>

namespace location
{
namespace glib
{

/// @brief SerializingBus implements Bus, dispatching messages via the global glib::Runtime instance.
class LOCATION_DLL_PUBLIC SerializingBus : public std::enable_shared_from_this<SerializingBus>, public Bus
{
public:
    /// @brief SerializingBus initializes a new instance with rt.
    static std::shared_ptr<SerializingBus> create();

    // From Bus
    void subscribe(const Event::Receiver::Ptr& receiver) override;
    void unsubscribe(const Event::Receiver::Ptr& receiver) override;
    void dispatch(const Event::Ptr& event) override;

private:
    /// @brief SerializingBus initializes a new instance with rt.
    SerializingBus();

    std::unordered_set<Event::Receiver::Ptr> receivers;
};

}  // namespace glib
}  // namespace location

#endif  // LOCATION_GLIB_SERIALIZING_BUS_H_
