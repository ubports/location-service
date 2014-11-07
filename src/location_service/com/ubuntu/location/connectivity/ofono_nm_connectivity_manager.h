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

#ifndef OFONO_NM_CONNECTIVITY_MANAGER_H_
#define OFONO_NM_CONNECTIVITY_MANAGER_H_

#include <com/ubuntu/location/connectivity/manager.h>

#include <com/ubuntu/location/logging.h>

#include "cached_radio_cell.h"
#include "cached_wireless_network.h"
#include "nm.h"
#include "ofono.h"

#include <core/dbus/bus.h>
#include <core/dbus/dbus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "../set_name_for_thread.h"

#include <boost/asio.hpp>

#include <chrono>

namespace dbus = core::dbus;

namespace com { namespace ubuntu { namespace location { namespace connectivity {
struct OfonoNmConnectivityManager : public com::ubuntu::location::connectivity::Manager
{
    // Creates an instance of the manager, resolving services on the given bus.
    OfonoNmConnectivityManager(const core::dbus::Bus::Ptr& bus);

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

    const core::Property<com::ubuntu::location::connectivity::Characteristics>& active_connection_characteristics() const;

    struct Private
    {
        Private(const core::dbus::Bus::Ptr& bus);
        ~Private();

        void setup_radio_stack_access();
        void on_modem_added(const core::dbus::types::ObjectPath& path);
        void on_modem_removed(const core::dbus::types::ObjectPath& path);
        void on_modem_interfaces_changed(const core::dbus::types::ObjectPath& path, const std::vector<std::string>& interfaces);

        // All network stack specific functionality goes here.
        void setup_network_stack_access();
        void on_device_added(const core::dbus::types::ObjectPath& device_path);
        void on_device_removed(const core::dbus::types::ObjectPath& device_path);
        void on_access_point_added(const core::dbus::types::ObjectPath& ap_path, const core::dbus::types::ObjectPath& device_path);
        void on_access_point_removed(const core::dbus::types::ObjectPath& ap_path);
        com::ubuntu::location::connectivity::Characteristics characteristics_for_connection(const core::dbus::types::ObjectPath& path);        

        core::dbus::Bus::Ptr bus;
        std::shared_ptr<core::dbus::DBus> bus_daemon;

        org::freedesktop::NetworkManager::Ptr network_manager;
        // If we failed to setup the network manager instance on startup,
        // we watch out for ownership changes on xdg::NetworkManager.
        std::shared_ptr<core::dbus::ServiceWatcher> network_manager_watcher;
        org::Ofono::Manager::Ptr modem_manager;
        // If we failed to setup the modem manager instance on startup,
        // we watch out for ownership changes on org.ofono.Manager.
        std::shared_ptr<core::dbus::ServiceWatcher> modem_manager_watcher;

        struct
        {
            // The io-service instance we use for dispatching invocations.
            boost::asio::io_service service;

            // We keep the io_service object alive until someone stops it.
            boost::asio::io_service::work keep_alive
            {
                service
            };

            // And a dedicated worker thread.
            std::thread worker
            {
                [this]() { service.run(); }
            };
        } dispatcher;

        struct
        {
            // We cache the properties as the network stack might not be
            // initialized after startup.
            core::Property<bool> is_wifi_enabled{false};
            core::Property<bool> is_wwan_enabled{false};
            core::Property<bool> is_wifi_hardware_enabled{false};
            core::Property<bool> is_wwan_hardware_enabled{false};

            // We guard access to all complex cached types with a mutex.
            mutable std::mutex guard;
            std::map<core::dbus::types::ObjectPath, detail::CachedRadioCell::Ptr> cells;
            std::map<core::dbus::types::ObjectPath, org::Ofono::Manager::Modem> modems;
            std::map<core::dbus::types::ObjectPath, detail::CachedWirelessNetwork::Ptr> wifis;
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::Device> wireless_devices;
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::ActiveConnection> primary_connection;
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::Device> primary_connection_devices;
        } cached;

        struct
        {
            core::Signal<> wireless_network_scan_finished;
            core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_added;
            core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr> connected_cell_removed;
            core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_added;
            core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr> wireless_network_removed;
        } signals;

        core::Property<com::ubuntu::location::connectivity::State> state;
        core::Property<com::ubuntu::location::connectivity::Characteristics> active_connection_characteristics;
    } d;
};
}}}}

#endif // OFONO_NM_CONNECTIVITY_MANAGER_H_
