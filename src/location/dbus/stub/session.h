/*
 * Copyright © 2012-2013 Canonical Ltd.
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

#ifndef LOCATION_DBUS_STUB_SESSION_H_
#define LOCATION_DBUS_STUB_SESSION_H_

#include <location/service.h>

#include <location/position.h>
#include <location/provider.h>
#include <location/result.h>
#include <location/update.h>

#include <location/dbus/session_gen.h>
#include <location/glib/shared_object.h>

#include <memory>

namespace location
{
namespace dbus
{
namespace stub
{
class Session : public location::Service::Session,
                public std::enable_shared_from_this<Session>
{
  public:
    using Ptr = std::shared_ptr<Session>;

    static void create(
            const glib::SharedObject<GDBusConnection>& connection,
            const std::string& path,
            std::function<void(const Result<Session::Ptr>&)> cb);

    ~Session();

    void start_position_updates();
    void stop_position_updates();

    void start_velocity_updates();
    void stop_velocity_updates();

    void start_heading_updates();
    void stop_heading_updates();

    Updates& updates() override;

  private:
    struct ProxyCreationContext
    {
        std::function<void(const location::Result<Ptr>&)> cb;
    };
    static void on_proxy_ready(GObject* source, GAsyncResult* res, gpointer user_data);

    static void on_position_changed(GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_heading_changed(GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_velocity_changed(GObject* object, GParamSpec* spec, gpointer user_data);

    Session(const glib::SharedObject<ComUbuntuLocationServiceSession>& session);

    Ptr finalize_construction();

    glib::SharedObject<ComUbuntuLocationServiceSession> session_;

    Updates updates_;
    std::set<ulong> signal_handler_ids_;
};
}
}
}

#endif // LOCATION_DBUS_STUB_SESSION_H_
