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
    return connectivity::State
    {
        static_cast<connectivity::State>(value)
    };
}

connectivity::Characteristics all_characteristics()
{
    return connectivity::Characteristics::connection_has_monetary_costs |
           connectivity::Characteristics::connection_is_bandwith_limited |
           connectivity::Characteristics::connection_is_volume_limited;
}
}

const core::Property<connectivity::State>& impl::OfonoNmConnectivityManager::state() const
{
    return d.state;
}

const core::Property<bool>& impl::OfonoNmConnectivityManager::is_wifi_enabled() const
{
    return *d.network_manager->properties.is_wifi_enabled;
}

const core::Property<bool>& impl::OfonoNmConnectivityManager::is_wwan_enabled() const
{
    return *d.network_manager->properties.is_wwan_enabled;
}

const core::Property<bool>& impl::OfonoNmConnectivityManager::is_wifi_hardware_enabled() const
{
    return *d.network_manager->properties.is_wifi_hardware_enabled;
}

const core::Property<bool>& impl::OfonoNmConnectivityManager::is_wwan_hardware_enabled() const
{
    return *d.network_manager->properties.is_wwan_hardware_enabled;
}

void impl::OfonoNmConnectivityManager::request_scan_for_wireless_networks()
{
    std::lock_guard<std::mutex> lg(d.cached.guard);

    for (const auto& pair : d.cached.wireless_devices)
        pair.second.request_scan();
}

const core::Signal<>& impl::OfonoNmConnectivityManager::wireless_network_scan_finished() const
{
    return d.signals.wireless_network_scan_finished;
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
    // We only report currently valid cells.
    for (const auto& cell : d.cached.cells)
        if (cell.second->is_valid().get())
            f(cell.second);
}

const core::Property<connectivity::Characteristics>& impl::OfonoNmConnectivityManager::active_connection_characteristics() const
{
    return d.active_connection_characteristics;
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
        SYSLOG(ERROR) << "Error while setting up access to radio and network stack: " << e.what();
    }
}

impl::OfonoNmConnectivityManager::Private::~Private()
{
    if (system_bus)
        system_bus->stop();

    if (worker.joinable())
        worker.join();

    dispatcher.service.stop();

    if (dispatcher.worker.joinable())
        dispatcher.worker.join();
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
            VLOG(1) << "Exception while creating connected radio cell: " << e.what();
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
            VLOG(1) << "Exception while adding modem: " << e.what();
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
            VLOG(1) << "Exception while removing modem: " << e.what();
        }
    });
}

void impl::OfonoNmConnectivityManager::Private::on_modem_added(const core::dbus::types::ObjectPath& path)
{
    auto modem = modem_manager->modem_for_path(path);

    // We immediately make the modem known to the cache, specifically
    // prior to attempting to create a connected cell.
    std::unique_lock<std::mutex> ul(cached.guard);
    auto modem_result = cached.modems.insert(std::make_pair(path, modem));

    // We immediate bail out if insertion into the cache does not succeed.
    if (not modem_result.second)
        return;

    // We first wire up to property changes here.
    modem_result.first->second.signals.property_changed->connect([this, path](const std::tuple<std::string, core::dbus::types::Variant>& tuple)
    {
        const auto& key = std::get<0>(tuple); VLOG(10) << "Property changed for modem: " << key;

        if (org::Ofono::Manager::Modem::Properties::Interfaces::name() == key)
        {
            auto interfaces = std::get<1>(tuple).as<std::vector<std::string> >();
            if (VLOG_IS_ON(10)) for(const auto& interface : interfaces) VLOG(10) << interface;
            on_modem_interfaces_changed(path, interfaces);
        }
    });

    // And update our cache of registered cells.
    auto cell_result = cached.cells.insert(
                std::make_pair(
                    modem.object->path(),
                    std::make_shared<detail::CachedRadioCell>(modem, dispatcher.service)));

    if (not cell_result.second)
        return;

    // We do not keep the cell alive.
    std::weak_ptr<detail::CachedRadioCell> wp{cell_result.first->second};

    // We account for a cell becoming invalid and report it as report.
    cell_result.first->second->is_valid().changed().connect([this, wp](bool valid)
    {
        VLOG(10) << "Validity of cell changed: " << std::boolalpha << valid << std::endl;

        auto sp = wp.lock();

        if (not sp)
            return;

        if (valid)
            signals.connected_cell_added(sp);
        else
            signals.connected_cell_removed(sp);
    });

    // Cool, we have reached here, updated all our caches and created a connected radio cell.
    // We are thus good to go and release the lock manually prior to announcing the new cell
    // to interested parties.
    ul.unlock();
    // Announce the newly added cell to API customers, without the lock
    // on the cache being held.
    signals.connected_cell_added(cell_result.first->second);
}

void impl::OfonoNmConnectivityManager::Private::on_modem_removed(const core::dbus::types::ObjectPath& path)
{
    detail::CachedRadioCell::Ptr cell;
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
        VLOG(1) << "Could not find a modem for path " << path.as_string();
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
        auto cell = std::make_shared<detail::CachedRadioCell>(itm->second, dispatcher.service);

        // We do not keep the cell alive.
        std::weak_ptr<detail::CachedRadioCell> wp{cell};

        // We account for a cell becoming invalid and report it as report.
        cell->is_valid().changed().connect([this, wp](bool valid)
        {
            VLOG(10) << "Validity of cell changed: " << std::boolalpha << valid << std::endl;

            auto sp = wp.lock();

            if (not sp)
                return;

            if (valid)
                signals.connected_cell_added(sp);
            else
                signals.connected_cell_removed(sp);
        });

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
            std::map<core::dbus::types::ObjectPath, org::freedesktop::NetworkManager::Device>::iterator it;
            std::tie(it, std::ignore) = cached.wireless_devices.insert(std::make_pair(device_path, device));

            // Iterate over all currently known wifis
            it->second.for_each_access_point([this, device_path](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_added(path, device_path);
                }
                catch (const std::exception& e)
                {
                    VLOG(1) << "Error while creating ap/wifi: " << e.what();
                }
            });

            it->second.signals.scan_done->connect([this]()
            {
                signals.wireless_network_scan_finished();
            });

            it->second.signals.ap_added->connect([this, device_path](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_added(path, device_path);
                }
                catch (const std::exception& e)
                {
                    VLOG(1) << "Error while creating ap/wifi: " << e.what();
                }
            });

            it->second.signals.ap_removed->connect([this](const core::dbus::types::ObjectPath& path)
            {
                try
                {
                    on_access_point_removed(path);
                }
                catch (const std::exception& e)
                {
                    VLOG(1) << "Error while removing ap/wifi: " << e.what();
                }
            });
        }
    });

    // Query the initial connectivity state
    state.set(from_nm_property(network_manager->properties.state->get()));
    // Determine the initial connection characteristics
    active_connection_characteristics.set(
                characteristics_for_connection(
                    network_manager->properties.primary_connection->get()));

    // And we wire up to property changes here
    network_manager->signals.properties_changed->connect([this](const std::map<std::string, core::dbus::types::Variant>& dict)
    {
        for (const auto& pair : dict)
        {
            VLOG(1) << "Property has changed: " << std::endl
                    << "  " << pair.first;

            if (xdg::NetworkManager::Properties::State::name() == pair.first)
            {
                state.set(from_nm_property(pair.second.as<xdg::NetworkManager::Properties::State::ValueType>()));
            }

            if (xdg::NetworkManager::Properties::PrimaryConnection::name() == pair.first)
            {
                // The primary connection object changed. We iterate over all the devices associated with
                // the primary connection and determine whether a wwan device is present. If so, we adjust
                // the characteristics of the connection to have monetary costs associated and to be
                // bandwidth and volume limited.

                auto path = pair.second.as<core::dbus::types::ObjectPath>();

                // We dispatch determining the connection characteristics to unblock
                // the bus here.
                dispatcher.service.post([this, path]()
                {
                    active_connection_characteristics = characteristics_for_connection(path);
                });
            }
        }
    });

    network_manager->signals.state_changed->connect([this](std::uint32_t s)
    {
        state.set(from_nm_property(s));
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

    auto wifi = std::make_shared<detail::CachedWirelessNetwork>(itd->second, ap);
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

connectivity::Characteristics impl::OfonoNmConnectivityManager::Private::characteristics_for_connection(const core::dbus::types::ObjectPath& path)
{
    xdg::NetworkManager::ActiveConnection ac
    {
        network_manager->service,
        network_manager->service->object_for_path(path)
    };

    connectivity::Characteristics characteristics
    {
        connectivity::Characteristics::none
    };

    // We try to enumerate all devices, and might fail if the active connection
    // went away under our feet. For that, we simply catch all possible exceptions
    // and silently drop them. In that case, we reset the characteristics to 'none'.
    try
    {
        ac.enumerate_devices([this, &characteristics](const core::dbus::types::ObjectPath& path)
        {
            xdg::NetworkManager::Device::Type type{xdg::NetworkManager::Device::Type::unknown};

            {
                std::lock_guard<std::mutex> lg{cached.guard};
                if (cached.wireless_devices.count(path) > 0)
                    type = cached.wireless_devices.at(path).type();
                else
                    type = xdg::NetworkManager::Device
                    {
                        network_manager->service,
                        network_manager->service->object_for_path(path)
                    }.type();
            }

            // We interpret a primary connection over a modem device as
            // having monetary costs (for the data plan), as well as being
            // bandwidth and volume limited. While this is not true in all
            // cases, it is good enough as a heuristic and for disabling certain
            // types of functionality if the data connection goes via a modem device.
            if (type == xdg::NetworkManager::Device::Type::modem)
            {
                characteristics = characteristics | connectivity::Characteristics::connection_goes_via_wwan;

                std::lock_guard<std::mutex> lg(cached.guard);

                for (const auto& pair : cached.modems)
                {
                    auto status = pair.second.network_registration.get<org::Ofono::Manager::Modem::NetworkRegistration::Status>();

                    if (org::Ofono::Manager::Modem::NetworkRegistration::Status::roaming == status)
                        characteristics = characteristics | connectivity::Characteristics::connection_is_roaming;
                }

                characteristics = characteristics | all_characteristics();
            } else if (type == xdg::NetworkManager::Device::Type::wifi)
            {
                characteristics = characteristics | connectivity::Characteristics::connection_goes_via_wifi;
            }

        });
    } catch(...)
    {
        // Empty on purpose.
    }

    return characteristics;
}

const std::shared_ptr<connectivity::Manager>& connectivity::platform_default_manager()
{
    static const std::shared_ptr<connectivity::Manager> instance{new impl::OfonoNmConnectivityManager{}};
    return instance;
}
