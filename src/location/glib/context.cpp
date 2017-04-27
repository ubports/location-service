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

#include <location/glib/context.h>

#include <location/result.h>
#include <location/dbus/stub/service.h>
#include <location/glib/runtime.h>

#include <thread>

namespace
{

class Context : public location::glib::Context
{
public:
    Context(location::dbus::Bus bus) : bus_{bus}
    {
    }

    void connect_to_service(const Callback& cb) override
    {
        location::dbus::stub::Service::create(bus_, [this, cb](const location::Result<location::dbus::stub::Service::Ptr>& result)
        {
            if (result)
                cb(result.value());
        });
    }

private:
    location::dbus::Bus bus_;
    location::glib::Runtime runtime_;
};

}  // namespace

std::shared_ptr<location::glib::Context> location::glib::Context::create_for_system_bus()
{
    return std::make_shared<::Context>(location::dbus::Bus::system);
}

std::shared_ptr<location::glib::Context> location::glib::Context::create_for_session_bus()
{
    return std::make_shared<::Context>(location::dbus::Bus::session);
}

location::glib::Context::~Context() = default;
location::glib::Context::Context() = default;
