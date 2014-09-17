/*
 * Copyright © 2014 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PROVIDER_DAEMON_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PROVIDER_DAEMON_H_

#include <com/ubuntu/location/provider.h>

#include <com/ubuntu/location/service/dbus_connection_factory.h>

#include <core/dbus/bus.h>
#include <core/dbus/object.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
// Provides a default out-of-process runner for known providers,
// leveraging the actual provider instance and a remote::Provider to
// expose it to the bus.
struct ProviderDaemon
{
    // Startup configuration goes here.
    struct Configuration
    {
        // Initializes a startup configuration from a command line.
        // Throws in case of issues.
        // Known parameters are:
        //   --bus={system, session}: The bus to connect to.
        //   --service-name=name: The name of the service under which the provider should be exposed.
        //   --service-path=path: The dbus object path under which the provider is known.
        //   --provider=name: The name of the actual provider implementation.
        static Configuration from_command_line_args(int argc, const char** argv, DBusConnectionFactory factory);

        // The bus connection that should be used by the remote::Provider::Skeleton instance.
        core::dbus::Bus::Ptr connection;
        // The object on the bus that represents the remote::Provider::Skeleton.
        core::dbus::Object::Ptr object;
        // The actual provider implementation.
        Provider::Ptr provider;
    };

    // Executes the daemon with the given configuration.
    static int main(const Configuration& configuration);
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PROVIDER_DAEMON_H_
