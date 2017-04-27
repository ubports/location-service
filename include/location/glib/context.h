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
#ifndef LOCATION_GLIB_CONTEXT_H_
#define LOCATION_GLIB_CONTEXT_H_

#include <location/service.h>
#include <location/visibility.h>

#include <functional>
#include <memory>

namespace location
{
namespace glib
{

/// @brief A Context bundles together runtime resources for
/// accessing a locationd service instance.
///
/// Note that both create_* functions assume that they are invoked
/// on a thread with a valid GMainContext. More to this, the resulting
/// Context instance will be associated with the GMainContext.
class LOCATION_DLL_PUBLIC Context
{
public:
    /// @brief Callback handles receiving of Service instances.
    typedef std::function<void(const Service::Ptr&)> Callback;

    /// @brief create_for_system_bus connects to a service instance
    /// on the system bus.
    static std::shared_ptr<Context> create_for_system_bus();

    /// @brief create_for_session_bus connects to a service instance
    /// on the session bus.
    static std::shared_ptr<Context> create_for_session_bus();

    /// @cond
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    virtual ~Context();
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
    /// @endcond

    /// @brief connect_to_service tries to establish a connection
    /// to a service instance.
    virtual void connect_to_service(const Callback& cb) = 0;

protected:
    /// @cond
    Context();
    /// @endcond
};

}  // namespace glib
}  // namespace location

#endif  // LOCATION_GLIB_CONTEXT_H_
