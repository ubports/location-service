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

#ifndef LOCATION_CONNECTIVITY_W11T_MANAGER_H_
#define LOCATION_CONNECTIVITY_W11T_MANAGER_H_

#include <location/connectivity/manager.h>

#include <location/connectivity/w11t_gen.h>

#include <location/result.h>
#include <location/glib/holder.h>
#include <location/glib/shared_object.h>

namespace location
{
namespace connectivity
{
namespace w11t
{

class BSS : public com::ubuntu::location::connectivity::WirelessNetwork,
            public std::enable_shared_from_this<BSS>
{
public:
    using Ptr = std::shared_ptr<BSS>;
    using Callback = std::function<void(const Result<Ptr>&)>;
    using Holder = glib::Holder<std::weak_ptr<BSS>>;

    static Mode mode_from_string(const std::string& mode);

    static void create(
            const glib::SharedObject<GDBusConnection>& connection,
            const std::string& path, const Callback& cb);

    std::string path() const;

    const core::Property<std::chrono::system_clock::time_point>& last_seen() const override;
    const core::Property<std::string>& bssid() const override;
    const core::Property<std::string>& ssid() const override;
    const core::Property<Mode>& mode() const override;
    const core::Property<Frequency>& frequency() const override;
    const core::Property<SignalStrength>& signal_strength() const override;

private:
    struct ProxyCreationContext
    {
        glib::SharedObject<GDBusConnection> connection;
        Callback cb;
    };
    static void on_proxy_ready(
            GObject* source_object, GAsyncResult* res, gpointer user_data);

    static void on_mode_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_frequency_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_signal_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);
    static void on_age_changed(
            GObject* object, GParamSpec* spec, gpointer user_data);

    BSS(const glib::SharedObject<GDBusConnection>& connection,
        const glib::SharedObject<FiW1Wpa_supplicant1BSS>& proxy);
    Ptr finalize_construction();

    glib::SharedObject<GDBusConnection> connection_;
    glib::SharedObject<FiW1Wpa_supplicant1BSS> proxy_;

    core::Property<std::string> bssid_;
    core::Property<std::string> ssid_;
    core::Property<Mode> mode_;
    core::Property<Frequency> frequency_;
    core::Property<SignalStrength> signal_strength_;
    core::Property<std::chrono::system_clock::time_point> last_seen_;
};

class Interface : public std::enable_shared_from_this<Interface>
{
public:
    using Ptr = std::shared_ptr<Interface>;
    using Callback = std::function<void(const Result<Ptr>&)>;
    using Holder = glib::Holder<std::weak_ptr<Interface>>;

    static void create(
            const glib::SharedObject<GDBusConnection>& connection,
            const std::string& path, const Callback& cb);

    void request_scan();

    std::string path() const;
    const core::Property<std::vector<BSS::Ptr>>& bsss() const;
    const core::Signal<bool>& scan_done() const;
    const core::Signal<BSS::Ptr>& bss_added() const;
    const core::Signal<BSS::Ptr>& bss_removed() const;

private:
    struct ProxyCreationContext
    {
        glib::SharedObject<GDBusConnection> connection;
        Callback cb;
    };
    static void on_proxy_ready(
            GObject* source_object, GAsyncResult* res, gpointer user_data);
    static void handle_scan_done(
            FiW1Wpasupplicant1WirelessInterface* object, gboolean arg_success, gpointer user_data);
    static void handle_bss_added(
            FiW1Wpasupplicant1WirelessInterface* object, const char* arg_path, GVariant* properties, gpointer user_data);
    static void handle_bss_removed(
            FiW1Wpasupplicant1WirelessInterface* object, const char* arg_path, gpointer user_data);

    Interface(const glib::SharedObject<GDBusConnection>& connection,
              const glib::SharedObject<FiW1Wpasupplicant1WirelessInterface>& proxy);
    Ptr finalize_construction();

    glib::SharedObject<GDBusConnection> connection_;
    glib::SharedObject<FiW1Wpasupplicant1WirelessInterface> proxy_;
    core::Property<std::vector<BSS::Ptr>> bsss_;
    core::Signal<bool> scan_done_;
    core::Signal<BSS::Ptr> bss_added_;
    core::Signal<BSS::Ptr> bss_removed_;
};

class Supplicant : public std::enable_shared_from_this<Supplicant>
{
public:
    using Ptr = std::shared_ptr<Supplicant>;
    using Callback = std::function<void(const Result<Ptr>&)>;
    using Holder = glib::Holder<std::weak_ptr<Supplicant>>;

    static constexpr const char* name()
    {
        return "fi.w1.wpa_supplicant1";
    }

    static constexpr const char* path()
    {
        return "/fi/w1/wpa_supplicant1";
    }

    static void create(const Callback& cb);

    const core::Property<std::vector<Interface::Ptr>>& interfaces() const;
    const core::Signal<Interface::Ptr>& interface_removed() const;
    const core::Signal<Interface::Ptr>& interface_added() const;

private:
    struct BusCreationContext
    {
        Callback cb;
    };
    static void on_bus_ready(
            GObject* source_object, GAsyncResult* res, gpointer user_data);

    struct ProxyCreationContext
    {
        glib::SharedObject<GDBusConnection> connection;
        Callback cb;
    };
    static void on_proxy_ready(
            GObject* source_object, GAsyncResult* res, gpointer user_data);

    static void handle_interface_added(
            FiW1Wpa_supplicant1* object, const char* arg_path, gpointer user_data);
    static void handle_interface_removed(
            FiW1Wpa_supplicant1* object, const char* arg_path, gpointer user_data);

    Supplicant(const glib::SharedObject<GDBusConnection>& connection,
               const glib::SharedObject<FiW1Wpa_supplicant1>& proxy);
    Ptr finalize_construction();

    glib::SharedObject<GDBusConnection> connection_;
    glib::SharedObject<FiW1Wpa_supplicant1> proxy_;
    core::Property<std::vector<Interface::Ptr>> interfaces_;
    core::Signal<Interface::Ptr> interface_added_;
    core::Signal<Interface::Ptr> interface_removed_;
};

class Manager : public com::ubuntu::location::connectivity::Manager,
                public std::enable_shared_from_this<Manager>
{
public:
    using Ptr = std::shared_ptr<Manager>;
    using Callback = std::function<void(const Result<Ptr>&)>;

    static void create(const Callback& cb);

    const core::Property<com::ubuntu::location::connectivity::State>& state() const override;
    const core::Property<bool>& is_wifi_enabled() const override;
    const core::Property<bool>& is_wwan_enabled() const override;
    const core::Property<bool>& is_wifi_hardware_enabled() const override;
    const core::Property<bool>& is_wwan_hardware_enabled() const override;

    void request_scan_for_wireless_networks() override;

    const core::Signal<>& wireless_network_scan_finished() const override;
    const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wireless_network_added() const override;
    const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& wireless_network_removed() const override;

    void enumerate_visible_wireless_networks(const std::function<void(const com::ubuntu::location::connectivity::WirelessNetwork::Ptr&)>& f) const override;

    const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& connected_cell_added() const override;
    const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& connected_cell_removed() const override;

    void enumerate_connected_radio_cells(const std::function<void(const com::ubuntu::location::connectivity::RadioCell::Ptr&)>& f) const override;

    const core::Property<com::ubuntu::location::connectivity::Characteristics>& active_connection_characteristics() const override;

private:
    Manager(const Supplicant::Ptr& supplicant);
    Ptr finalize_construction();

    Supplicant::Ptr supplicant_;

    core::Property<com::ubuntu::location::connectivity::State> state_;
    core::Property<bool> is_wifi_enabled_;
    core::Property<bool> is_wwan_enabled_;
    core::Property<bool> is_wifi_hardware_enabled_;
    core::Property<bool> is_wwan_hardware_enabled_;
    core::Property<com::ubuntu::location::connectivity::Characteristics> active_connection_characteristics_;

    core::Signal<> wireless_network_scan_finished_;
    core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_added_;
    core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_removed_;

    core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_added_;
    core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_removed_;


};

}  // namespace w11t
}  // namespace connectivity
}  // namespace location

#endif  // LOCATION_CONNECTIVITY_W11T_MANAGER_H_
