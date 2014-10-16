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

#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_DISPATCHING_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_DISPATCHING_PROVIDER_H_

#include <com/ubuntu/location/provider.h>

#include <functional>
#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
// A Provider implementation that wraps another provider implementation
// dispatching events/invocations via a Dispatcher functor. The dispatcher
// can either immediately process the given task or hand it over to a runtime
// with an associated event loop.
class DispatchingProvider : public Provider, public std::enable_shared_from_this<DispatchingProvider>
{
public:
    // To safe us some typing.
    typedef std::shared_ptr<DispatchingProvider> Ptr;

    // The Dispatcher functor that is invoked for all incoming
    // invocations and for all events, with both of them being
    // wrapped as a task.
    typedef std::function<void()> Task;
    typedef std::function<void(Task)> Dispatcher;

    DispatchingProvider(const Dispatcher& dispatcher, const Provider::Ptr& fwd);
    ~DispatchingProvider() noexcept;

    bool supports(const location::Provider::Features& f) const override;
    bool requires(const location::Provider::Requirements& r) const override;
    bool matches_criteria(const location::Criteria&) override;

    // We forward all events to the other providers.
    void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state) override;
    void on_reference_location_updated(const location::Update<location::Position>& position) override;
    void on_reference_velocity_updated(const location::Update<location::Velocity>& velocity) override;
    void on_reference_heading_updated(const location::Update<location::Heading>& heading) override;

    // As well as the respective state change requests.
    void start_position_updates() override;
    void stop_position_updates() override;
    void start_heading_updates() override;
    void stop_heading_updates() override;
    void start_velocity_updates() override;
    void stop_velocity_updates() override;

private:
    // The dispatcher we rely on to dispatch events/invocations.
    Dispatcher dispatcher;
    // The provider that we relay to/from.
    Provider::Ptr fwd;
    // We store all connections that should be cut on destruction.
    struct
    {
        core::ScopedConnection position_updates;
        core::ScopedConnection heading_updates;
        core::ScopedConnection velocity_updates;
    } connections;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_DISPATCHING_PROVIDER_H_
