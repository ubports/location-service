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
 * Authored by: Manuel de la Peña <manuel.delapena@canonical.com>
 */

#include <location/providers/remote/provider.h>

#include <location/dbus/codec.h>
#include <location/dbus/util.h>
#include <location/glib/holder.h>
#include <location/glib/runtime.h>
#include <location/glib/util.h>

#include <location/events/registry.h>
#include <location/events/reference_position_updated.h>
#include <location/events/wifi_and_cell_id_reporting_state_changed.h>
#include <location/logging.h>

#include <core/posix/this_process.h>

#include <thread>

namespace cul = location;
namespace remote = location::providers::remote;

namespace
{

using SkeletonHolder = location::glib::Holder<std::weak_ptr<remote::Provider::Skeleton>>;
using Holder = location::glib::Holder<std::weak_ptr<remote::Provider::Stub>>;

}  // namespace

void remote::Provider::Stub::create(
        const glib::SharedObject<GDBusConnection>& connection,
        const std::string& name,
        const std::string& path,
        std::function<void(const Result<Provider::Ptr>&)> cb)
{
    com_ubuntu_location_service_provider_proxy_new(
                connection.get(), G_DBUS_PROXY_FLAGS_NONE, name.c_str(), path.c_str(),
                nullptr, on_proxy_ready, new OnProxyReadyContext{std::move(cb)});
}

void remote::Provider::Stub::on_proxy_ready(
        GObject* source, GAsyncResult* res, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(source);

    if (auto context = static_cast<OnProxyReadyContext*>(user_data))
    {
        GError* error{nullptr};
        auto proxy = com_ubuntu_location_service_provider_proxy_new_finish(res, &error);

        if (error)
        {
            context->cb(location::make_error_result<location::Provider::Ptr>(
                glib::wrap_error_as_exception(error)));
        }
        else
        {
            std::shared_ptr<remote::Provider::Stub> sp{new remote::Provider::Stub{
                        location::glib::make_shared_object(proxy)
                }};
            context->cb(location::make_result<location::Provider::Ptr>(sp->finalize_construction()));
        }

        delete context;
    }
}

remote::Provider::Stub::Stub(const glib::SharedObject<ComUbuntuLocationServiceProvider>& stub)
    : stub_{stub},
      requirements_{*dbus::decode<Requirements>(
                        com_ubuntu_location_service_provider_get_requirements(stub_.get()))}
{
}

std::shared_ptr<remote::Provider::Stub> remote::Provider::Stub::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Stub> wp{sp};

    g_signal_connect_data(stub_.get(), "notify::requirements",
                          G_CALLBACK(on_requirements_changed),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(stub_.get(), "notify::position",
                          G_CALLBACK(on_position_updated),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(stub_.get(), "notify::heading",
                          G_CALLBACK(on_heading_updated),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(stub_.get(), "notify::velocity",
                          G_CALLBACK(on_velocity_updated),
                          new Holder{wp}, Holder::closure_notify, GConnectFlags(0));

    return sp;
}

remote::Provider::Stub::~Stub() noexcept
{
}

void remote::Provider::Stub::on_new_event(const Event& event)
{
    com_ubuntu_location_service_provider_call_on_new_event(stub_.get(), dbus::encode(event), nullptr, nullptr, nullptr);
}

void remote::Provider::Stub::enable()
{
    com_ubuntu_location_service_provider_call_enable(stub_.get(), nullptr, nullptr, nullptr);
}

void remote::Provider::Stub::disable()
{
    com_ubuntu_location_service_provider_call_disable(stub_.get(), nullptr, nullptr, nullptr);
}

void remote::Provider::Stub::activate()
{
    com_ubuntu_location_service_provider_call_activate(stub_.get(), nullptr, nullptr, nullptr);
}

void remote::Provider::Stub::deactivate()
{
    com_ubuntu_location_service_provider_call_deactivate(stub_.get(), nullptr, nullptr, nullptr);
}

location::Provider::Requirements remote::Provider::Stub::requirements() const
{
    return requirements_;
}

bool remote::Provider::Stub::satisfies(const Criteria& criteria)
{
    gboolean result{false};
    if (!com_ubuntu_location_service_provider_call_satisfies_sync(stub_.get(), dbus::encode(criteria), &result, nullptr, nullptr))
        return false;
    return result;
}

const core::Signal<location::Update<location::Position>>& remote::Provider::Stub::position_updates() const
{
    return position_updates_;
}

const core::Signal<location::Update<location::units::Degrees>>& remote::Provider::Stub::heading_updates() const
{
    return heading_updates_;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& remote::Provider::Stub::velocity_updates() const
{
    return velocity_updates_;
}

void remote::Provider::Stub::on_requirements_changed(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto holder = static_cast<Holder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            auto requirements =
                    dbus::decode<Requirements>(
                        com_ubuntu_location_service_provider_get_requirements(
                            sp->stub_.get()));
            if (requirements)
                sp->requirements_ = *requirements;
        }
    }
}

void remote::Provider::Stub::on_position_updated(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto thiz = static_cast<Holder*>(user_data))
    {
        if (auto sp = thiz->value.lock())
        {
            auto update =
                    dbus::decode<location::Update<location::Position>>(com_ubuntu_location_service_provider_get_position(
                                                                           sp->stub_.get()));
            if (update)
                sp->position_updates_(*update);
        }
    }
}

void remote::Provider::Stub::on_heading_updated(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto thiz = static_cast<Holder*>(user_data))
    {
        if (auto sp = thiz->value.lock())
        {
            auto update =
                    dbus::decode<location::Update<units::Degrees>>(com_ubuntu_location_service_provider_get_heading(
                                                                       sp->stub_.get()));
            if (update)
                sp->heading_updates_(*update);
        }
    }
}

void remote::Provider::Stub::on_velocity_updated(GObject* object, GParamSpec* spec, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(object, spec);

    if (auto thiz = static_cast<Holder*>(user_data))
    {
        if (auto sp = thiz->value.lock())
        {
            auto update =
                    dbus::decode<location::Update<location::units::MetersPerSecond>>(com_ubuntu_location_service_provider_get_velocity(
                                                                                         sp->stub_.get()));
            if (update)
                sp->velocity_updates_(*update);
        }
    }
}

location::Provider::Ptr remote::Provider::Skeleton::create(const glib::SharedObject<GDBusConnection>& connection,
                                        const std::string& path,
                                        const Provider::Ptr& impl)
{
    auto skeleton = glib::make_shared_object(com_ubuntu_location_service_provider_skeleton_new());

    GError* error{nullptr};
    if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton.get()), connection.get(), path.c_str(), &error))
        std::rethrow_exception(glib::wrap_error_as_exception(error));

    std::shared_ptr<remote::Provider::Skeleton> sp{new remote::Provider::Skeleton{skeleton, impl}};
    return sp->finalize_construction();
}

remote::Provider::Skeleton::Skeleton(const glib::SharedObject<ComUbuntuLocationServiceProvider>& skeleton,
                                     const Provider::Ptr& impl)
    : skeleton_{skeleton},
      impl_{impl}
{    
}

std::shared_ptr<remote::Provider::Skeleton> remote::Provider::Skeleton::finalize_construction()
{
    auto sp = shared_from_this();
    std::weak_ptr<Skeleton> wp{sp};

    g_signal_connect_data(skeleton_.get(), "handle-on-new-event",
                          G_CALLBACK(Skeleton::handle_on_new_event),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(skeleton_.get(), "handle-satisfies",
                          G_CALLBACK(Skeleton::handle_satisfies),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(skeleton_.get(), "handle-enable",
                          G_CALLBACK(Skeleton::handle_enable),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(skeleton_.get(), "handle-disable",
                          G_CALLBACK(Skeleton::handle_disable),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(skeleton_.get(), "handle-activate",
                          G_CALLBACK(Skeleton::handle_activate),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    g_signal_connect_data(skeleton_.get(), "handle-deactivate",
                          G_CALLBACK(Skeleton::handle_deactivate),
                          new SkeletonHolder{wp}, SkeletonHolder::closure_notify, GConnectFlags(0));

    impl_->position_updates().connect([this, wp](const Update<Position>& value)
    {
        glib::Runtime::instance()->dispatch([this, wp, value]()
        {
            if (auto sp = wp.lock())
                com_ubuntu_location_service_provider_set_position(skeleton_.get(), dbus::encode(value));
        });
    });

    impl_->heading_updates().connect([this, wp](const Update<units::Degrees>& value)
    {
        glib::Runtime::instance()->dispatch([this, wp, value]()
        {
            if (auto sp = wp.lock())
                com_ubuntu_location_service_provider_set_heading(skeleton_.get(), dbus::encode(value));
        });
    });

    impl_->velocity_updates().connect([this, wp](const Update<units::MetersPerSecond>& value)
    {
        glib::Runtime::instance()->dispatch([this, wp, value]()
        {
            if (auto sp = wp.lock())
                com_ubuntu_location_service_provider_set_velocity(skeleton_.get(), dbus::encode(value));
        });
    });

    return sp;
}

remote::Provider::Skeleton::~Skeleton() noexcept
{
}

// We just forward calls to the actual implementation
bool remote::Provider::Skeleton::satisfies(const cul::Criteria& criteria)
{
    return impl_->satisfies(criteria);
}

location::Provider::Requirements remote::Provider::Skeleton::requirements() const
{
    return impl_->requirements();
}

void remote::Provider::Skeleton::on_new_event(const Event& e)
{
    impl_->on_new_event(e);
}

void remote::Provider::Skeleton::enable()
{
    impl_->enable();
}

void remote::Provider::Skeleton::disable()
{
    impl_->disable();
}

void remote::Provider::Skeleton::activate()
{
    impl_->activate();
}

void remote::Provider::Skeleton::deactivate()
{
    impl_->deactivate();
}

const core::Signal<location::Update<location::Position>>& remote::Provider::Skeleton::position_updates() const
{
    return impl_->position_updates();
}

const core::Signal<location::Update<location::units::Degrees>>& remote::Provider::Skeleton::heading_updates() const
{
    return impl_->heading_updates();
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& remote::Provider::Skeleton::velocity_updates() const
{
    return impl_->velocity_updates();
}

bool remote::Provider::Skeleton::handle_on_new_event(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, GVariant* event, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            if (auto decoded_event = dbus::decode<Event::Ptr>(event))
            {
                sp->on_new_event(*(decoded_event.get()));
                com_ubuntu_location_service_provider_complete_on_new_event(provider, invocation);
                return true;
            }
        }
    }

    return false;
}

bool remote::Provider::Skeleton::handle_satisfies(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, GVariant* criteria, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            if (auto decoded_criteria = dbus::decode<Criteria>(criteria))
            {
                bool satisfies = sp->satisfies(*decoded_criteria);
                com_ubuntu_location_service_provider_complete_satisfies(provider, invocation, satisfies);
                return true;
            }
        }
    }

    return false;
}

bool remote::Provider::Skeleton::handle_enable(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->enable();
            com_ubuntu_location_service_provider_complete_enable(provider, invocation);
            return true;
        }
    }

    return false;
}

bool remote::Provider::Skeleton::handle_disable(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->disable();
            com_ubuntu_location_service_provider_complete_disable(provider, invocation);
            return true;
        }
    }

    return false;
}

bool remote::Provider::Skeleton::handle_activate(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->activate();
            com_ubuntu_location_service_provider_complete_activate(provider, invocation);
            return true;
        }
    }

    return false;
}

bool remote::Provider::Skeleton::handle_deactivate(
        ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data)
{
    LOCATION_DBUS_TRACE_STATIC_TRAMPOLIN;
    boost::ignore_unused(provider);

    if (auto holder = static_cast<SkeletonHolder*>(user_data))
    {
        if (auto sp = holder->value.lock())
        {
            sp->deactivate();
            com_ubuntu_location_service_provider_complete_deactivate(provider, invocation);
            return true;
        }
    }

    return false;
}
