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

#include <location/connectivity/w11t_manager.h>

#include <location/dbus/util.h>
#include <location/glib/util.h>

#include <boost/format.hpp>

namespace w11t = location::connectivity::w11t;

namespace
{
std::string byte_array_to_mac_address(const std::uint8_t* data, std::size_t size)
{
    std::stringstream ss;
    for (std::size_t i = 0; i < size; i++)
        ss << (i > 0 ? ":" : "") << (boost::format("%02x") % std::uint32_t(data[i])).str();

    return ss.str();
}
}

w11t::BSS::Mode w11t::BSS::mode_from_string(const std::string& mode)
{
    if (mode == "ad-hoc")
        return Mode::adhoc;
    else if (mode == "infrastructure")
        return Mode::infrastructure;
    return Mode::unknown;
}

void w11t::BSS::create(
        const glib::SharedObject<GDBusConnection>& connection,
        const std::string& path, const std::function<void(const Result<Ptr>&)>& cb)
{
    fi_w1_wpa_supplicant1_bss_proxy_new(
                connection.get(), G_DBUS_PROXY_FLAGS_NONE, Supplicant::name(), path.c_str(),
                nullptr, on_proxy_ready, new ProxyCreationContext{connection, cb});
}

std::string w11t::BSS::path() const
{
    char* object_path{nullptr};
    g_object_get(proxy_.get(), "g-object-path", &object_path, nullptr);
    return std::string{object_path};
}

const core::Property<std::chrono::system_clock::time_point>& w11t::BSS::last_seen() const
{
    return last_seen_;
}

const core::Property<std::string>& w11t::BSS::bssid() const
{
    return bssid_;
}

const core::Property<std::string>& w11t::BSS::ssid() const
{
    return ssid_;
}

const core::Property<w11t::BSS::Mode>& w11t::BSS::mode() const
{
    return mode_;
}

const core::Property<w11t::BSS::Frequency>& w11t::BSS::frequency() const
{
    return frequency_;
}

const core::Property<w11t::BSS::SignalStrength>& w11t::BSS::signal_strength() const
{
    return signal_strength_;
}

void w11t::BSS::on_proxy_ready(
        GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source_object);

    if (auto context = static_cast<ProxyCreationContext*>(user_data))
    {
        GError* error{nullptr};
        auto proxy = fi_w1_wpa_supplicant1_bss_proxy_new_finish(res, &error);

        if (error)
        {
            context->cb(make_error_result<Ptr>(glib::wrap_error_as_exception(error)));
        }
        else
        {
            try
            {
                Ptr sp{new BSS{context->connection, glib::make_shared_object(proxy)}};
                context->cb(make_result(sp->finalize_construction()));
            }
            catch (...)
            {
                context->cb(std::current_exception());
            }
        }

        delete context;
    }
}

void w11t::BSS::on_mode_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
            sp->mode_ = BSS::mode_from_string(fi_w1_wpa_supplicant1_bss_get_mode(sp->proxy_.get()));
    }
}

void w11t::BSS::on_frequency_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
            sp->frequency_ = Frequency(fi_w1_wpa_supplicant1_bss_get_frequency(sp->proxy_.get()));
    }
}

void w11t::BSS::on_signal_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
            sp->signal_strength_ = SignalStrength(fi_w1_wpa_supplicant1_bss_get_signal(sp->proxy_.get()));
    }
}

void w11t::BSS::on_age_changed(
        GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
            sp->last_seen_ = std::chrono::system_clock::now() - std::chrono::seconds{
                    fi_w1_wpa_supplicant1_bss_get_age(sp->proxy_.get())};
    }
}

w11t::BSS::BSS(const glib::SharedObject<GDBusConnection>& connection,
               const glib::SharedObject<FiW1Wpa_supplicant1BSS>& proxy)
    : connection_{connection},
      proxy_{proxy}
{
    
    auto variant = fi_w1_wpa_supplicant1_bss_get_bssid(proxy_.get());
    if (variant)
    {
        gsize elements{0}; auto data = g_variant_get_fixed_array(variant, &elements, sizeof(guint8));
        if (data)
        {
            bssid_.set(byte_array_to_mac_address(reinterpret_cast<const std::uint8_t*>(data), elements));
        }
        else
        {
            throw std::runtime_error{"Missing bssid"};
        }
    }
    variant = fi_w1_wpa_supplicant1_bss_get_ssid(proxy_.get());
    if (variant)
    {
        gsize elements{0}; auto data = g_variant_get_fixed_array(variant, &elements, sizeof(guint8));
        if (data)
            ssid_.set(std::string(reinterpret_cast<const char*>(data), elements));
    }

    mode_.set(mode_from_string(fi_w1_wpa_supplicant1_bss_get_mode(proxy_.get())));
    frequency_.set(Frequency(fi_w1_wpa_supplicant1_bss_get_frequency(proxy_.get())));
    signal_strength_.set(SignalStrength(fi_w1_wpa_supplicant1_bss_get_signal(proxy_.get())));
    last_seen_.set(std::chrono::system_clock::now() - std::chrono::seconds(fi_w1_wpa_supplicant1_bss_get_age(proxy_.get())));
}

w11t::BSS::Ptr w11t::BSS::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<BSS> wp{sp};

    g_signal_connect_data(G_OBJECT(proxy_.get()), "notify::mode",
                          G_CALLBACK(on_mode_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "notify::frequency",
                          G_CALLBACK(on_frequency_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "notify::signal",
                          G_CALLBACK(on_signal_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "notify::age",
                          G_CALLBACK(on_age_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    return sp;
}

void w11t::Interface::create(
        const glib::SharedObject<GDBusConnection>& connection,
        const std::string& path, const Callback& cb)
{
    fiw1wpa_supplicant1wirelessinterface_proxy_new(
                connection.get(), G_DBUS_PROXY_FLAGS_NONE, Supplicant::name(), path.c_str(),
                nullptr, on_proxy_ready, new ProxyCreationContext{connection, cb});
}

void w11t::Interface::request_scan()
{
    GVariantBuilder builder; g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&builder, "{sv}", "Type", g_variant_new_string("active"));

    fiw1wpa_supplicant1wirelessinterface_call_scan(
                proxy_.get(), g_variant_builder_end(&builder), nullptr, nullptr, nullptr);
}

std::string w11t::Interface::path() const
{
    char* object_path{nullptr};
    g_object_get(proxy_.get(), "g-object-path", &object_path, nullptr);
    return std::string{object_path};
}

const core::Property<std::vector<w11t::BSS::Ptr>>& w11t::Interface::bsss() const
{
    return bsss_;
}

const core::Signal<bool>& w11t::Interface::scan_done() const
{
    return scan_done_;
}

const core::Signal<w11t::BSS::Ptr>& w11t::Interface::bss_added() const
{
    return bss_added_;
}

const core::Signal<w11t::BSS::Ptr>& w11t::Interface::bss_removed() const
{
    return bss_removed_;
}

void w11t::Interface::on_proxy_ready(
        GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source_object);

    if (auto context = static_cast<ProxyCreationContext*>(user_data))
    {
        GError* error{nullptr};
        auto proxy = fiw1wpa_supplicant1wirelessinterface_proxy_new_finish(res, &error);

        if (error)
        {
            context->cb(make_error_result<Ptr>(glib::wrap_error_as_exception(error)));
        }
        else
        {
            Ptr sp{new Interface{context->connection, glib::make_shared_object(proxy)}};
            context->cb(make_result(sp->finalize_construction()));
        }

        delete context;
    }
}

void w11t::Interface::handle_scan_done(
        FiW1Wpasupplicant1WirelessInterface* object, gboolean arg_success, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->scan_done_(arg_success);
        }
    }
}

void w11t::Interface::handle_bss_added(
        FiW1Wpasupplicant1WirelessInterface* object, const char* arg_path, GVariant* properties, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, properties);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            BSS::create(sp->connection_, arg_path, [sp](const Result<BSS::Ptr>& result)
            {
                if (result)
                {
                    auto bss = result.value();

                    sp->bsss_.update([bss](std::vector<BSS::Ptr>& value) -> bool
                    {
                        value.push_back(bss);
                        return true;
                    });

                    sp->bss_added_(bss);
                }
            });
        }
    }
}

void w11t::Interface::handle_bss_removed(
        FiW1Wpasupplicant1WirelessInterface* object, const char* arg_path, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            BSS::Ptr removed;
            sp->bsss_.update([arg_path, &removed](std::vector<BSS::Ptr>& value) -> bool
            {
                auto it = std::find_if(value.begin(), value.end(), [arg_path](const BSS::Ptr& bss)
                {
                    return bss->path() == arg_path;
                });

                if (it != value.end())
                {
                    removed = *it;
                    value.erase(it);
                }

                return it != value.end();
            });

            if (removed)
                sp->bss_removed_(removed);
        }
    }
}

w11t::Interface::Interface(const glib::SharedObject<GDBusConnection>& connection,
                           const glib::SharedObject<FiW1Wpasupplicant1WirelessInterface>& proxy)
    : connection_{connection},
      proxy_{proxy}
{
}

w11t::Interface::Ptr w11t::Interface::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Interface> wp{sp};

    g_signal_connect_data(G_OBJECT(proxy_.get()), "scan-done",
                          G_CALLBACK(handle_scan_done),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "bssadded",
                          G_CALLBACK(handle_bss_added),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "bssremoved",
                          G_CALLBACK(handle_bss_removed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    auto iterator = fiw1wpa_supplicant1wirelessinterface_get_bsss(proxy_.get());
    while (iterator && *iterator)
    {
        BSS::create(connection_, *iterator, [this, sp](const Result<BSS::Ptr>& result)
        {
            if (result)
            {
                bsss_.update([bss = result.value()](std::vector<BSS::Ptr>& value) -> bool
                {
                    value.push_back(bss);
                    return true;
                });
            }
        });

        ++iterator;
    }

    return sp;
}

void w11t::Supplicant::create(const Callback& cb)
{
    g_bus_get(G_BUS_TYPE_SYSTEM, nullptr, on_bus_ready, new BusCreationContext{cb});
}

const core::Property<std::vector<w11t::Interface::Ptr>>& w11t::Supplicant::interfaces() const
{
    return interfaces_;
}

const core::Signal<w11t::Interface::Ptr>& w11t::Supplicant::interface_removed() const
{
    return interface_removed_;
}

const core::Signal<w11t::Interface::Ptr>& w11t::Supplicant::interface_added() const
{
    return interface_added_;
}

void w11t::Supplicant::on_bus_ready(
        GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source_object);

    if (auto context = static_cast<BusCreationContext*>(user_data))
    {
        GError* error{nullptr};
        auto connection = glib::make_shared_object(g_bus_get_finish(res, &error));

        if (error)
        {
            context->cb(make_error_result<Ptr>(glib::wrap_error_as_exception(error)));
        }
        else
        {
            fi_w1_wpa_supplicant1_proxy_new(
                        connection.get(), G_DBUS_PROXY_FLAGS_NONE, Supplicant::name(), Supplicant::path(),
                        nullptr, on_proxy_ready, new ProxyCreationContext{connection, context->cb});
        }
        delete context;
    }
}

void w11t::Supplicant::on_proxy_ready(
        GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source_object);

    if (auto context = static_cast<ProxyCreationContext*>(user_data))
    {
        GError* error{nullptr};
        auto proxy = fi_w1_wpa_supplicant1_proxy_new_finish(res, &error);

        if (error)
        {
            context->cb(make_error_result<Ptr>(glib::wrap_error_as_exception(error)));
        }
        else
        {
            Ptr sp{new Supplicant{context->connection, glib::make_shared_object(proxy)}};
            context->cb(make_result(sp->finalize_construction()));
        }

        delete context;
    }
}

void w11t::Supplicant::handle_interface_added(
        FiW1Wpa_supplicant1* object, const char* arg_path, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            Interface::create(sp->connection_, arg_path, [sp](const Result<Interface::Ptr>& result)
            {
                if (result)
                {
                    auto interface = result.value();

                    sp->interfaces_.update([interface](std::vector<Interface::Ptr>& value) -> bool
                    {
                        value.push_back(interface);
                        return true;
                    });

                    sp->interface_added_(interface);
                }
            });
        }
    }
}

void w11t::Supplicant::handle_interface_removed(
        FiW1Wpa_supplicant1* object, const char* arg_path, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            Interface::Ptr removed;
            sp->interfaces_.update([arg_path, &removed](std::vector<Interface::Ptr>& value) -> bool
            {
                auto it = std::find_if(value.begin(), value.end(), [arg_path](const Interface::Ptr& interface)
                {
                    return interface->path() == arg_path;
                });

                if (it != value.end())
                {
                    removed = *it;
                    value.erase(it);
                }

                return it != value.end();
            });

            if (removed)
                sp->interface_removed_(removed);
        }
    }
}

w11t::Supplicant::Supplicant(const glib::SharedObject<GDBusConnection>& connection,
                             const glib::SharedObject<FiW1Wpa_supplicant1>& proxy)
    : connection_{connection},
      proxy_{proxy}
{
}

w11t::Supplicant::Ptr w11t::Supplicant::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Supplicant> wp{sp};

    g_signal_connect_data(G_OBJECT(proxy_.get()), "interface-added",
                          G_CALLBACK(handle_interface_removed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(G_OBJECT(proxy_.get()), "interface-removed",
                          G_CALLBACK(handle_interface_added),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    auto iterator = fi_w1_wpa_supplicant1_get_interfaces(proxy_.get());

    while (*iterator)
    {
        Interface::create(connection_, *iterator, [this, sp](const Result<Interface::Ptr>& result)
        {
            if (result)
            {
                auto interface = result.value();

                interfaces_.update([interface](std::vector<Interface::Ptr>& value) -> bool
                {
                    value.push_back(interface);
                    return true;
                });

                interface_added_(interface);
            }
        });

        ++iterator;
    }

    return sp;
}

void w11t::Manager::create(const Callback& cb)
{
    Supplicant::create([cb](const Result<Supplicant::Ptr>& result)
    {
        if (result)
        {
            Ptr sp{new Manager{result.value()}};
            cb(make_result(sp->finalize_construction()));
        }
        else
        {
            try
            {
                result.rethrow();
            }
            catch (...)
            {
                cb(make_error_result<Ptr>(std::current_exception()));
            }
        }
    });
}

w11t::Manager::Manager(const Supplicant::Ptr& supplicant)
    : supplicant_{supplicant},
      state_{com::ubuntu::location::connectivity::State::connected_global},
      is_wifi_enabled_{true},
      is_wwan_enabled_{false},
      is_wifi_hardware_enabled_{true},
      is_wwan_hardware_enabled_{false},
      active_connection_characteristics_{com::ubuntu::location::connectivity::Characteristics::connection_goes_via_wifi}
{
}

w11t::Manager::Ptr w11t::Manager::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Manager> wp{sp};

    for (const auto& interface : supplicant_->interfaces().get())
    {
        interface->bss_added().connect([this, wp](const BSS::Ptr& bss)
        {
            if (auto sp = wp.lock())
                wireless_network_added_(bss);
        });

        interface->bss_removed().connect([this, wp](const BSS::Ptr& bss)
        {
            if (auto sp = wp.lock())
                wireless_network_removed_(bss);
        });

        interface->scan_done().connect([this, wp](bool success)
        {
            if (!success)
                return;

            if (auto sp = wp.lock())
                wireless_network_scan_finished_();
        });
    }

    supplicant_->interface_added().connect([this, wp](const Interface::Ptr& interface)
    {
        interface->bss_added().connect([this, wp](const BSS::Ptr& bss)
        {
            if (auto sp = wp.lock())
                wireless_network_added_(bss);
        });

        interface->bss_removed().connect([this, wp](const BSS::Ptr& bss)
        {
            if (auto sp = wp.lock())
                wireless_network_removed_(bss);
        });

        interface->scan_done().connect([this, wp](bool success)
        {
            if (!success)
                return;

            if (auto sp = wp.lock())
                wireless_network_scan_finished_();
        });
    });

    return sp;
}

const core::Property<com::ubuntu::location::connectivity::State>& w11t::Manager::state() const
{
    return state_;
}

const core::Property<bool>& w11t::Manager::is_wifi_enabled() const
{
    return is_wifi_enabled_;
}

const core::Property<bool>& w11t::Manager::is_wwan_enabled() const
{
    return is_wwan_enabled_;
}

const core::Property<bool>& w11t::Manager::is_wifi_hardware_enabled() const
{
    return is_wifi_hardware_enabled_;
}

const core::Property<bool>& w11t::Manager::is_wwan_hardware_enabled() const
{
    return is_wwan_hardware_enabled_;
}

void w11t::Manager::request_scan_for_wireless_networks()
{
    for (const auto& interface : supplicant_->interfaces().get())
        interface->request_scan();
}

const core::Signal<>& w11t::Manager::wireless_network_scan_finished() const
{
    return wireless_network_scan_finished_;
}

const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& w11t::Manager::wireless_network_added() const
{
    return wireless_network_added_;
}

const core::Signal<com::ubuntu::location::connectivity::WirelessNetwork::Ptr>& w11t::Manager::wireless_network_removed() const
{
    return wireless_network_removed_;
}

void w11t::Manager::enumerate_visible_wireless_networks(const std::function<void(const com::ubuntu::location::connectivity::WirelessNetwork::Ptr&)>& f) const
{
    for (const auto& interface : supplicant_->interfaces().get())
        for (const auto& bss : interface->bsss().get())
            f(bss);
}

const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& w11t::Manager::connected_cell_added() const
{
    return connected_cell_added_;
}

const core::Signal<com::ubuntu::location::connectivity::RadioCell::Ptr>& w11t::Manager::connected_cell_removed() const
{
    return connected_cell_removed_;
}

void w11t::Manager::enumerate_connected_radio_cells(const std::function<void(const com::ubuntu::location::connectivity::RadioCell::Ptr&)>& f) const
{
    boost::ignore_unused(f);
}

const core::Property<com::ubuntu::location::connectivity::Characteristics>& w11t::Manager::active_connection_characteristics() const
{
    return active_connection_characteristics_;
}

void com::ubuntu::location::connectivity::platform_default_manager(const std::function<void(const std::shared_ptr<Manager>&)>& cb)
{
    w11t::Manager::create([cb](const ::location::Result<w11t::Manager::Ptr>& result)
    {
        if (result)
            cb(result.value());
    });
}
