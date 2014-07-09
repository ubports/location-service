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
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
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

namespace impl
{
struct OfonoNmConnectivityManager : public com::ubuntu::location::connectivity::Manager
{
    const core::Property<com::ubuntu::location::connectivity::State>& state() const override;

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
        Private();
        ~Private();

        void setup_radio_stack_access();
        void on_modem_added(const core::dbus::types::ObjectPath& path);
        void on_modem_removed(const core::dbus::types::ObjectPath& path);
        void on_modem_interfaces_changed(const core::dbus::types::ObjectPath& path, const std::vector<std::string>& interfaces);

        void setup_network_stack_access();
        void on_access_point_added(const core::dbus::types::ObjectPath& ap_path, const core::dbus::types::ObjectPath& device_path);
        void on_access_point_removed(const core::dbus::types::ObjectPath& ap_path);

        core::dbus::Bus::Ptr system_bus;
        core::dbus::Executor::Ptr executor;

        std::thread worker;

        org::freedesktop::NetworkManager::Ptr network_manager;
        org::Ofono::Manager::Ptr modem_manager;

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
            mutable std::mutex guard;
            std::map<core::dbus::types::ObjectPath, CachedRadioCell::Ptr> cells;
            std::map<core::dbus::types::ObjectPath, org::Ofono::Manager::Modem> modems;
            std::map<core::dbus::types::ObjectPath, CachedWirelessNetwork::Ptr> wifis;
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
}

#endif // OFONO_NM_CONNECTIVITY_MANAGER_H_
