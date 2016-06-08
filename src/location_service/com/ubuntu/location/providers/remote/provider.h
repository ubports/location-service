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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_REMOTE_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_REMOTE_PROVIDER_H_

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_factory.h>

#include <com/ubuntu/location/providers/remote/skeleton.h>
#include <com/ubuntu/location/providers/remote/stub.h>

#include <core/dbus/bus.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace remote
{
struct Provider
{
    class Stub : public com::ubuntu::location::Provider, public std::enable_shared_from_this<Stub>
    {
    public:
        // For integration with the Provider factory.
        static std::string class_name();

        // Instantiates a new provider instance, populating the configuration object
        // from the provided property bundle.
        static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

        // Creates a new provider instance with the given config object.
        static Provider::Ptr create_instance_with_config(const stub::Configuration& config);

        // Name of the command line parameter for passing in the DBus to connect to.
        static constexpr const char* key_bus{"bus"};
        // Name of the command line parameter for passing in the remote service name.
        static constexpr const char* key_name{"name"};
        // Name of the command line parameter for passing in the path of the remote provider impl.
        static constexpr const char* key_path{"path"};

        ~Stub() noexcept;

        virtual bool matches_criteria(const Criteria&) override;
        virtual bool supports(const Features& f) const override;
        virtual bool requires(const Requirements& r) const override;

        virtual void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state) override;
        virtual void on_reference_location_updated(const Update<Position>& position) override;
        virtual void on_reference_velocity_updated(const Update<Velocity>& velocity) override;
        virtual void on_reference_heading_updated(const Update<Heading>& heading) override;

        virtual void start_position_updates() override;
        virtual void stop_position_updates() override;

        virtual void start_heading_updates() override;
        virtual void stop_heading_updates() override;

        virtual void start_velocity_updates() override;
        virtual void stop_velocity_updates() override;

        // Returns the name of the remote provider on the bus (useful for fusioning)
        std::string get_dbus_path();

    private:
        Stub(const stub::Configuration& config);

        // Yeah, two stage init is evil.
        void setup_event_connections();
        // ping tries to reach out to the remote end and throws
        // if the ping fails.
        void ping();

        struct Private;
        std::shared_ptr<Private> d;
    };

    class Skeleton : public com::ubuntu::location::Provider
    {
    public:
        Skeleton(const remote::skeleton::Configuration& config);
        ~Skeleton() noexcept;

        virtual bool matches_criteria(const Criteria&) override;

        virtual bool supports(const Features& f) const override;
        virtual bool requires(const Requirements& r) const override;

        virtual void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state) override;
        virtual void on_reference_location_updated(const Update<Position>& position) override;
        virtual void on_reference_velocity_updated(const Update<Velocity>& velocity) override;
        virtual void on_reference_heading_updated(const Update<Heading>& heading) override;

        virtual void start_position_updates() override;
        virtual void stop_position_updates() override;

        virtual void start_heading_updates() override;
        virtual void stop_heading_updates() override;

        virtual void start_velocity_updates() override;
        virtual void stop_velocity_updates() override;

    private:
        struct Private;
        std::shared_ptr<Private> d;
    };
};
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_REMOTE_PROVIDER_H_
