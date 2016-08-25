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

#include <location/providers/remote/skeleton.h>
#include <location/providers/remote/stub.h>

#include <core/dbus/bus.h>
#include <core/dbus/object.h>

namespace location
{
namespace providers
{
namespace remote
{
struct Provider
{
    class Observer
    {
    public:
        virtual ~Observer() = default;

        virtual void on_new_position(const Update<Position>&) = 0;
        virtual void on_new_heading(const Update<Heading>&) = 0;
        virtual void on_new_velocity(const Update<Velocity>&) = 0;

        class Stub;
        class Skeleton;
    };

    class Observer::Stub : public Observer
    {
    public:
        explicit Stub(const core::dbus::Object::Ptr& object);

        // From Observer
        void on_new_position(const Update<Position>&) override;
        void on_new_heading(const Update<Heading>&) override;
        void on_new_velocity(const Update<Velocity>&) override;

    private:
        core::dbus::Object::Ptr object;
    };

    class Observer::Skeleton : public Observer
    {
    public:
        explicit Skeleton(const core::dbus::Bus::Ptr& bus, const core::dbus::Object::Ptr& object, const std::shared_ptr<Observer>& impl);

        // From Observer
        void on_new_position(const Update<Position>&) override;
        void on_new_heading(const Update<Heading>&) override;
        void on_new_velocity(const Update<Velocity>&) override;

    private:
        core::dbus::Bus::Ptr bus;
        core::dbus::Object::Ptr object;
        std::shared_ptr<Observer> impl;
    };

    class Stub : public location::Provider, public Observer, public std::enable_shared_from_this<Stub>
    {
    public:
        // Creates a new provider instance with the given config object.
        static void create_instance_with_config(const stub::Configuration& config, const std::function<void(const Provider::Ptr&)>& cb);

        ~Stub() noexcept;

        void add_observer(const std::shared_ptr<Observer>& observer);

        // From Observer
        void on_new_position(const Update<Position>&) override;
        void on_new_heading(const Update<Heading>&) override;
        void on_new_velocity(const Update<Velocity>&) override;

        // From location::Provider
        void on_new_event(const Event& event) override;
        void enable() override;
        void disable() override;
        void activate() override;
        void deactivate() override;

        Requirements requirements() const override;
        bool satisfies(const Criteria& criteria) override;
        const core::Signal<Update<Position>>& position_updates() const override;
        const core::Signal<Update<Heading>>& heading_updates() const override;
        const core::Signal<Update<Velocity>>& velocity_updates() const override;

    private:
        Stub(const stub::Configuration& config, Requirements requirements);

        // Yeah, two stage init is evil.
        std::shared_ptr<Stub> finalize();

        struct Private;
        std::shared_ptr<Private> d;
    };

    class Skeleton : public location::Provider
    {
    public:
        Skeleton(const remote::skeleton::Configuration& config);
        ~Skeleton() noexcept;

        void add_observer(const std::shared_ptr<Observer>& observer);

        void on_new_event(const Event& event) override;

        void enable() override;
        void disable() override;
        void activate() override;
        void deactivate() override;

        Requirements requirements() const override;
        bool satisfies(const Criteria& criteria) override;
        const core::Signal<Update<Position>>& position_updates() const override;
        const core::Signal<Update<Heading>>& heading_updates() const override;
        const core::Signal<Update<Velocity>>& velocity_updates() const override;

    private:
        struct Private;
        std::shared_ptr<Private> d;
    };
};
}
}
}

#endif // LOCATION_PROVIDERS_REMOTE_PROVIDER_H_
