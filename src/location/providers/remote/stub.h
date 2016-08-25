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
#ifndef LOCATION_SERVICE_PROVIDERS_REMOTE_STUB_H_
#define LOCATION_SERVICE_PROVIDERS_REMOTE_STUB_H_

#include <location/provider.h>

#include <functional>

namespace core { namespace dbus {
class Bus;
class Object;
class Service;
}}

namespace location
{
namespace providers
{
namespace remote
{
namespace stub
{
/** @brief All creation time arguments go here. */
struct Configuration
{
    /** @brief Bus connection to answer incoming calls. */
    std::shared_ptr<core::dbus::Bus> bus;
    /** @brief Remote service instance. */
    std::shared_ptr<core::dbus::Service> service;
    /** @brief Remote object implementing remote::Interface. */
    std::shared_ptr<core::dbus::Object> object;
};

/// @brief Asynchronously creates a stub instance referring to a remote provider instance.
void create_with_configuration(const Configuration& configuration, const std::function<void(const Provider::Ptr&)>& cb);
}
}
}
}

#endif // LOCATION_SERVICE_PROVIDERS_REMOTE_STUB_H_
