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
#ifndef LOCATION_DBUS_STUB_SERVICE_H_
#define LOCATION_DBUS_STUB_SERVICE_H_

#include <location/service.h>

#include <location/result.h>
#include <location/dbus/bus.h>
#include <location/dbus/service.h>
#include <location/dbus/service_gen.h>
#include <location/glib/shared_object.h>

#include <gio/gio.h>

namespace location
{
namespace dbus
{
namespace stub
{
class Service : public location::Service, public std::enable_shared_from_this<Service>
{
  public:
    using Ptr = std::shared_ptr<Service>;

    static void create(Bus bus, std::function<void(const Result<Service::Ptr>&)> callback);

    ~Service();

    // From location::Service
    void create_session_for_criteria(const Criteria& criteria, const std::function<void(const Session::Ptr&)>& cb) override;
    void add_provider(const Provider::Ptr &provider) override;
    const core::Property<State>& state() const override;
    core::Property<bool>& does_satellite_based_positioning() override;
    core::Property<bool>& does_report_cell_and_wifi_ids() override;
    core::Property<bool>& is_online() override;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() override;

  private:
    struct BusAcquisitionContext
    {
        std::function<void(const Result<Service::Ptr>&)> cb;
    };
    static void on_bus_acquired(GObject* source, GAsyncResult* res, gpointer user_data);

    struct NameAppearedForCreationContext
    {
        guint watch_id;
        std::function<void(const Result<Service::Ptr>&)> cb;
    };
    static void on_name_appeared_for_creation(
            GDBusConnection* connection_, const gchar* name, const gchar* name_owner, gpointer user_data);

    struct ProxyCreationContext
    {
        glib::SharedObject<GDBusConnection> connection;
        Service::Ptr instance;
        std::function<void(const Result<Service::Ptr>&)> cb;
    };
    static void on_proxy_ready(GObject* source, GAsyncResult* res, gpointer user_data);

    struct SessionCreationContext
    {
        std::function<void(const Session::Ptr&)> cb;
        std::weak_ptr<Service> wp;
    };
    static void on_session_ready(GObject *source, GAsyncResult *res, gpointer user_data);

    struct ProviderAdditionContext
    {
        Provider::Ptr provider;
        std::weak_ptr<Service> wp;
    };
    static void on_provider_added(GObject* source, GAsyncResult* res, gpointer user_data);

    static void on_does_satellite_based_positioning_changed(GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_does_report_cell_and_wifi_ids_changed(GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_is_online_changed(GObject* object, GParamSpec* spec, gpointer user_data);

    Service();

    void deinit();
    std::shared_ptr<Service> init(const glib::SharedObject<GDBusConnection>& connection_,
                                  const glib::SharedObject<ComUbuntuLocationService>& service);

    glib::SharedObject<GDBusConnection> connection_;
    glib::SharedObject<ComUbuntuLocationService> proxy_;
    std::set<ulong> signal_handler_ids;

    std::uint64_t provider_counter{0};
    std::set<Provider::Ptr> providers;

    core::Property<Service::State> state_;
    core::Property<bool> does_satellite_based_positioning_;
    core::Property<bool> does_report_cell_and_wifi_ids_;
    core::Property<bool> is_online_;
    core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>> visible_space_vehicles_;
};
}
}
}

#endif // LOCATION_DBUS_STUB_SERVICE_H_
