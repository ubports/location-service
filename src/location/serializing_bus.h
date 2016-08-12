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
#ifndef LOCATION_SERIALIZING_BUS_H_
#define LOCATION_SERIALIZING_BUS_H_

#include <location/bus.h>
#include <location/runtime.h>

#include <boost/asio.hpp>

#include <unordered_set>

namespace location
{
/// @brief SerializingBus implements Bus, dispatching messages via a boost::asio::io_service strand.
class SerializingBus : public std::enable_shared_from_this<SerializingBus>, public Bus
{
public:
    /// @brief SerializingBus initializes a new instance with rt.
    static std::shared_ptr<SerializingBus> create(const std::shared_ptr<Runtime>& rt);

    // From Bus
    void subscribe(const Event::Receiver::Ptr& receiver) override;
    void unsubscribe(const Event::Receiver::Ptr& receiver) override;
    void dispatch(const Event::Ptr& event) override;

private:
    /// @brief SerializingBus initializes a new instance with rt.
    SerializingBus(const std::shared_ptr<Runtime>& rt);

    std::shared_ptr<Runtime> rt;
    boost::asio::io_service::strand strand;
    std::unordered_set<Event::Receiver::Ptr> receivers;
};
}

#endif // LOCATION_SERIALIZING_BUS_H_
