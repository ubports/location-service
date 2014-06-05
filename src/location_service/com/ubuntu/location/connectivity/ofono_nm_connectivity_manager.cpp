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

#include "ofono_nm_connectivity_manager.h"

namespace connectivity = com::ubuntu::location::connectivity;
namespace dbus = core::dbus;
namespace xdg = org::freedesktop;
namespace
{
connectivity::State from_nm_property(std::uint32_t value)
{
    connectivity::State result{connectivity::State::unknown};

    switch (value)
    {
    case xdg::NetworkManager::Properties::Connectivity::Values::unknown:
        result = connectivity::State::unknown;
        break;
    case xdg::NetworkManager::Properties::Connectivity::Values::none:
        result = connectivity::State::none;
        break;
    case xdg::NetworkManager::Properties::Connectivity::Values::portal:
        result = connectivity::State::portal;
        break;
    case xdg::NetworkManager::Properties::Connectivity::Values::limited:
        result = connectivity::State::limited;
        break;
    case xdg::NetworkManager::Properties::Connectivity::Values::full:
        result = connectivity::State::full;
        break;
    }

    return result;
}
}

const core::Property<connectivity::State>& impl::OfonoNmConnectivityManager::state() const
{
    return d.state;
}

void impl::OfonoNmConnectivityManager::request_scan_for_wireless_networks()
{
    std::lock_guard<std::mutex> lg(d.cached.guard);

    for (const auto& pair : d.cached.wireless_devices)
        pair.second.request_scan();
}

const core::Signal<connectivity::WirelessNetwork::Ptr>& impl::OfonoNmConnectivityManager::wireless_network_added() const
{
    return d.signals.wireless_network_added;
}

const core::Signal<connectivity::WirelessNetwork::Ptr>& impl::OfonoNmConnectivityManager::wireless_network_removed() const
{
    return d.signals.wireless_network_removed;
}

void impl::OfonoNmConnectivityManager::enumerate_visible_wireless_networks(const std::function<void(const connectivity::WirelessNetwork::Ptr&)>& f) const
{
    std::lock_guard<std::mutex> lg(d.cached.guard);
    for (const auto& wifi : d.cached.wifis)
        f(wifi.second);
}

const core::Signal<connectivity::RadioCell::Ptr>& impl::OfonoNmConnectivityManager::connected_cell_added() const
{
    return d.signals.connected_cell_added;
}

const core::Signal<connectivity::RadioCell::Ptr>& impl::OfonoNmConnectivityManager::connected_cell_removed() const
{
    return d.signals.connected_cell_removed;
}

void impl::OfonoNmConnectivityManager::enumerate_connected_radio_cells(const std::function<void(const connectivity::RadioCell::Ptr&)>& f) const
{
    std::lock_guard<std::mutex> lg(d.cached.guard);
    for (const auto& cell : d.cached.cells)
        f(cell.second);
}

impl::OfonoNmConnectivityManager::Private::Private()
{
    try
    {
        system_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);
        executor = dbus::asio::make_executor(system_bus);
        system_bus->install_executor(executor);

        worker = std::move(std::thread
        {
           [this]()
           {
               system_bus->run();
           }
        });

        setup_network_stack_access();
        setup_radio_stack_access();
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Error while setting up access to radio and network stack: " << e.what();
    }
}

impl::OfonoNmConnectivityManager::Private::~Private()
{
    if (system_bus)
        system_bus->stop();

    if (worker.joinable())
        worker.join();
}

void impl::OfonoNmConnectivityManager::Private::setup_radio_stack_access()
{
    modem_manager.reset(new org::Ofono::Manager(system_bus));

    modem_manager->for_each_modem([this](const core::dbus::types::ObjectPath& path)
    {
        try
        {
            on_modem_added(path);
        }
        catch(const std::runtime_error& e)
        {
            LOG(WARNING) << "Exception while creating connected radio cell: " << e.what();
        }
    });

    modem_manager->signals.modem_added->connect([this](const org::Ofono::Manager::ModemAdded::ArgumentType& arg)
    {
        try
        {
            on_modem_added(std::get<0>(arg));
        }
        catch(const std::exception& e)
        {
            LOG(WARNING) << "Exception while adding modem: " << e.what();
        }
    });

    modem_manager->signals.modem_removed->connect([this](const core::dbus::types::ObjectPath& path)
    {
        try
        {
            on_modem_removed(path);
        }
        catch(const std::exception& e)
        {
            LOG(WARNING) << "Exception while removing modem: " << e.what();
        }
    });
}

void impl::OfonoNmConnectivityManager::Private::on_modem_added(const core::dbus::types::ObjectPath& path)
{
    auto modem = modem_manager->modem_for_path(path);

    // We first wire up to property changes here.
    modem.signals.property_changed->connect([this, path](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
    {
        const auto& key = std::get<0>(tuple); VLOG(10) << "Property changed for modem: " << key;

        if (org::Ofono::Manager::Modem::Properties::Interfaces::name() == key)
        {
            auto interfaces = std::get<1>(tuple).as<std::vector<std::string> >();
            if (VLOG_IS_ON(10)) for(const auto& interface : interfaces) VLOG(10) << interface;
            on_modem_interfaces_changed(path, interfaces);
        }

    });

    // And update our cache of modems and registered cells.
    auto cell = std::make_shared<CachedRadioCell>(modem);
    {
        std::lock_guard<std::mutex> lg(cached.guard);
        cached.modems.insert(std::make_pair(modem.object->path(), modem));
        cached.cells.insert(std::make_pair(modem.object->path(), cell));
    }
    // Announce the newly added cell to API customers, without the lock
    // on the cache being held.
    signals.connected_cell_added(cell);
}

void impl::OfonoNmConnectivityManager::Private::on_modem_removed(const core::dbus::types::ObjectPath& path)
{
    CachedRadioCell::Ptr cell;
    {
        std::lock_guard<std::mutex> lg(cached.guard);

        // Update modem and cell cache.
        auto itc = cached.cells.find(path);
        auto itm = cached.modems.find(path);

        if (itc != cached.cells.end())
        {
            cell = itc->second;
            cached.cells.erase(itc);
        }

        if (itm != cached.modems.end())
        {
            cached.modems.erase(path);
        }
    }
    // Inform customers of the registered cell being removed, without
    // the lock on the cache being held.
    if (cell) signals.connected_cell_removed(cell);
}

void impl::OfonoNmConnectivityManager::Private::on_modem_interfaces_changed(
        const core::dbus::types::ObjectPath& path,
        const std::vector<std::string>& interfaces)
{
    std::unique_lock<std::mutex> ul(cached.guard);

    auto itm = cached.modems.find(path);
    if (itm == cached.modems.end())
    {
        LOG(WARNING) << "Could not find a modem for path " << path.as_string();
        return;
    }

    auto itt = cached.cells.find(path);
    const bool has_cell_for_modem = itt != cached.cells.end();

    auto it = std::find(
                interfaces.begin(),
                interfaces.end(),
                std::string{org::Ofono::Manager::Modem::Properties::Interfaces::network_registration});
    const bool modem_has_network_registration = it != interfaces.end();

    if (has_cell_for_modem and not modem_has_network_registration)
    {
        // A network registration was lost and we remove the corresponding
        // cell instance from the cache.
        auto cell = itt->second;
        cached.cells.erase(itt);

        // Cache is up to date now and we announce the removal of the cell
        // to API customers, with the lock on the cache not being held.
        ul.unlock(); signals.connected_cell_removed(cell);
    } else if (not has_cell_for_modem and modem_has_network_registration)
    {
        // A new network registration is coming in and we have to create
        // a corresponding cell instance.
        auto cell = std::make_shared<CachedRadioCell>(itm->second);
        cached.cells.insert(std::make_pair(path,cell));
        // Cache is up to date now and we announce the new cell to
        // API customers, with the lock on the cache not being held.
        ul.unlock(); signals.connected_cell_added(cell);
    }
}

void impl::OfonoNmConnectivityManager::Private::setup_network_stack_access()
{
    network_manager.reset(new xdg::NetworkManager(system_bus));

    network_manager->for_each_device([this](const core::dbus::types::ObjectPath& device_path)
    {
        auto device = network_manager->device_for_path(device_path);

        if (device.type() == xdg::NetworkManager::Device::Type::wifi)
        {
            // Make the device known to the cache.
            cached.wireless_devices.insert(std::make_pair(device_path, device));

            // Iterate over all currently known wifis
            device.for_each_access_point([this, device_path](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_added(path, device_path);
                }
                catch (const std::exception& e)
                {
                    LOG(ERROR) << "Error while creating ap/wifi: " << e.what();
                }
            });

            device.signals.ap_added->connect([this, device_path](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_added(path, device_path);
                }
                catch (const std::exception& e)
                {
                    LOG(ERROR) << "Error while creating ap/wifi: " << e.what();
                }
            });

            device.signals.ap_removed->connect([this](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_removed(path);
                }
                catch (const std::exception& e)
                {
                    LOG(ERROR) << "Error while removing ap/wifi: " << e.what();
                }
            });
        }
    });

    // Query the initial connectivity state
    state.set(from_nm_property(network_manager->properties.connectivity->get()));

    // And we wire up to property changes here
    network_manager->signals.properties_changed->connect([this](const std::map<std::string, core::dbus::types::Variant>& dict)
    {
        for (const auto& pair : dict)
        {
            VLOG(1) << "Property has changed: " << std::endl
                    << "  " << pair.first;

            if (xdg::NetworkManager::Properties::Connectivity::name() == pair.first)
            {
                state.set(from_nm_property(pair.second.as<xdg::NetworkManager::Properties::Connectivity::ValueType>()));
            }
        }
    });
}

void impl::OfonoNmConnectivityManager::Private::on_access_point_added(
        const core::dbus::types::ObjectPath& ap_path,
        const core::dbus::types::ObjectPath& device_path)
{
    std::unique_lock<std::mutex> ul(cached.guard);

    // Let's see if we have a device known for the path. We return early
    // if we do not know about the device.
    auto itd = cached.wireless_devices.find(device_path);
    if (itd == cached.wireless_devices.end())
        return;

    xdg::NetworkManager::AccessPoint ap
    {
        network_manager->service->add_object_for_path(ap_path)
    };

    auto wifi = std::make_shared<CachedWirelessNetwork>(itd->second, ap);
    cached.wifis[ap_path] = wifi;

    // Let API consumers know that an AP appeared. The lock on the cache is
    // not held to prevent from deadlocks.
    ul.unlock(); signals.wireless_network_added(wifi);
}

void impl::OfonoNmConnectivityManager::Private::on_access_point_removed(const core::dbus::types::ObjectPath& ap_path)
{
    std::unique_lock<std::mutex> ul(cached.guard);

    // Let's see if we know about the wifi. We return early if not.
    auto itw = cached.wifis.find(ap_path);
    if (itw == cached.wifis.end())
        return;

    // Update the cache and keep the wifi object alive until API consumers
    // have been informed of the wifi going away.
    connectivity::WirelessNetwork::Ptr wifi = itw->second;
    cached.wifis.erase(itw);

    // Let API consumers know that an AP disappeared. The lock on the cache is
    // not held to prevent from deadlocks.
    ul.unlock(); signals.wireless_network_removed(wifi);
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new impl::OfonoNmConnectivityManager{}};
    return instance;
}
