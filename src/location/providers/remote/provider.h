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
#ifndef LOCATION_PROVIDERS_REMOTE_PROVIDER_H_
#define LOCATION_PROVIDERS_REMOTE_PROVIDER_H_

#include <location/provider.h>
#include <location/provider_factory.h>
#include <location/result.h>
#include <location/visibility.h>

#include <location/glib/shared_object.h>

#include <location/dbus/provider_gen.h>

namespace location
{
namespace providers
{
namespace remote
{
struct LOCATION_DLL_PUBLIC Provider
{
    static constexpr const char* name()
    {
        return "com.ubuntu.location.Service.Provider";
    }

    class Stub : public location::Provider, public std::enable_shared_from_this<Stub>
    {
    public:
        using Ptr = std::shared_ptr<Stub>;

        // Asynchronously tries to create a new Stub instance, setting up proxy access to the remote
        // end. Reports errors via 'cb'.
        static void create(const glib::SharedObject<GDBusConnection>& connection,
                           const std::string& name,
                           const std::string& path,
                           std::function<void(const Result<Provider::Ptr>&)> cb);

        ~Stub() noexcept;

        // From location::Provider
        void on_new_event(const Event& event) override;
        void enable() override;
        void disable() override;
        void activate() override;
        void deactivate() override;

        Requirements requirements() const override;
        bool satisfies(const Criteria& criteria) override;

        const core::Signal<Update<Position>>& position_updates() const override;
        const core::Signal<Update<units::Degrees>>& heading_updates() const override;
        const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

    private:
        struct OnProxyReadyContext
        {
            std::function<void(const location::Result<location::Provider::Ptr>&)> cb;
        };

        static void on_proxy_ready(GObject* source_object, GAsyncResult* res, gpointer user_data);
        static void on_requirements_changed(GObject* object, GParamSpec* spec, gpointer user_data);
        static void on_position_updated(GObject* object, GParamSpec* spec, gpointer user_data);
        static void on_heading_updated(GObject* object, GParamSpec* spec, gpointer user_data);
        static void on_velocity_updated(GObject* object, GParamSpec* spec, gpointer user_data);

        Stub(const glib::SharedObject<ComUbuntuLocationServiceProvider>& stub);
        std::shared_ptr<Stub> finalize_construction();

        glib::SharedObject<ComUbuntuLocationServiceProvider> stub_;
        Requirements requirements_;
        core::Signal<Update<Position>> position_updates_;
        core::Signal<Update<units::Degrees>> heading_updates_;
        core::Signal<Update<units::MetersPerSecond>> velocity_updates_;
    };

    class Skeleton : public location::Provider, public std::enable_shared_from_this<Skeleton>
    {
    public:
        using Ptr = std::shared_ptr<Skeleton>;

        // Asynchronously tries to create a new Skeleton instance, setting up
        // proxy access to the remote end. Reports errors via cb.
        static Provider::Ptr create(
                const glib::SharedObject<GDBusConnection>& connection,
                const std::string& path,
                const Provider::Ptr& impl);

        ~Skeleton() noexcept;

        // From location::Provider
        void on_new_event(const Event& event) override;
        void enable() override;
        void disable() override;
        void activate() override;
        void deactivate() override;

        Requirements requirements() const override;
        bool satisfies(const Criteria& criteria) override;

        const core::Signal<Update<Position>>& position_updates() const override;
        const core::Signal<Update<units::Degrees>>& heading_updates() const override;
        const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const override;

    private:
        static bool handle_on_new_event(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, GVariant* event, gpointer user_data);
        static bool handle_satisfies(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, GVariant* requirements, gpointer user_data);
        static bool handle_enable(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data);
        static bool handle_disable(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data);
        static bool handle_activate(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data);
        static bool handle_deactivate(
                ComUbuntuLocationServiceProvider* provider, GDBusMethodInvocation* invocation, gpointer user_data);

        Skeleton(const glib::SharedObject<ComUbuntuLocationServiceProvider>& skeleton,
                 const Provider::Ptr& impl);

        std::shared_ptr<Skeleton> finalize_construction();

        glib::SharedObject<ComUbuntuLocationServiceProvider> skeleton_;
        Provider::Ptr impl_;
    };
};
}
}
}

#endif // LOCATION_PROVIDERS_REMOTE_PROVIDER_H_
