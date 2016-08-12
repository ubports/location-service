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

#include <location/serializing_bus.h>

std::shared_ptr<location::SerializingBus> location::SerializingBus::create(const std::shared_ptr<Runtime>& rt)
{
    return std::shared_ptr<location::SerializingBus>{new location::SerializingBus{rt}};
}

location::SerializingBus::SerializingBus(const std::shared_ptr<Runtime>& rt)
    : rt{rt}, strand{rt->service()}
{
}

void location::SerializingBus::subscribe(const Event::Receiver::Ptr& receiver)
{
    auto thiz = shared_from_this();
    strand.dispatch([this, thiz, receiver]() { receivers.insert(receiver); });
}

void location::SerializingBus::unsubscribe(const Event::Receiver::Ptr& receiver)
{
    auto thiz = shared_from_this();
    strand.dispatch([this, thiz, receiver]() { receivers.erase(receiver); });
}

void location::SerializingBus::dispatch(const Event::Ptr& event)
{
    auto thiz = shared_from_this();
    strand.dispatch([this, thiz, event]() { for (const auto& rec : receivers) rec->on_new_event(*event); });
}
