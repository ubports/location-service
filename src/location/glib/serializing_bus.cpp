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

#include <location/glib/serializing_bus.h>

#include <location/glib/runtime.h>

std::shared_ptr<location::glib::SerializingBus> location::glib::SerializingBus::create()
{
    return std::shared_ptr<location::glib::SerializingBus>{new location::glib::SerializingBus{}};
}

location::glib::SerializingBus::SerializingBus()
{
}

void location::glib::SerializingBus::subscribe(const Event::Receiver::Ptr& receiver)
{
    auto thiz = shared_from_this();

    glib::Runtime::instance()->dispatch([this, thiz, receiver]()
    {
        receivers.insert(receiver);
    });
}

void location::glib::SerializingBus::unsubscribe(const Event::Receiver::Ptr& receiver)
{
    auto thiz = shared_from_this();

    glib::Runtime::instance()->dispatch([this, thiz, receiver]()
    {
        receivers.erase(receiver);
    });
}

void location::glib::SerializingBus::dispatch(const Event::Ptr& event)
{
    auto thiz = shared_from_this();

    glib::Runtime::instance()->dispatch([this, thiz, event]()
    {
        for (const auto& receiver : receivers)
            receiver->on_new_event(*event);
    });
}
