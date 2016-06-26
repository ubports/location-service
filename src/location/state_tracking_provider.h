/*
 * Copyright © 2016 Canonical Ltd.
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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_STATE_TRACKING_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_STATE_TRACKING_PROVIDER_H_

#include <location/provider.h>

#include <memory>

namespace location
{
// StateTrackingProvider keeps track of the State of an individual provider instance.
//
// TODO(tvoss): Factor this functionality into the public interface once we find time
// to break API/ABI of providers.
class StateTrackingProvider : public Provider
{
public:
    // Safe us some typing
    typedef std::shared_ptr<StateTrackingProvider> Ptr;

    // State models the different states of a provider.
    enum class State
    {
        active, // The provider is actively delivering updates.
        enabled // The provider is enabled but not actively delivering updates.
    };

    StateTrackingProvider(const Provider::Ptr& impl)
        : impl_{impl},
          connections
          {
              impl_->updates().position.connect(
                  [this](const Update<Position>& u)
                  {
                      mutable_updates().position(u);
                  }),
              impl_->updates().heading.connect(
                  [this](const Update<Heading>& u)
                  {
                      mutable_updates().heading(u);
                  }),
              impl_->updates().velocity.connect(
                  [this](const Update<Velocity>& u)
                  {
                      mutable_updates().velocity(u);
                  })
           },
          state_{State::enabled}
    {
    }

    const core::Property<State>& state() const
    {
        return state_;
    }

    bool supports(const Features& f) const override
    {
        return impl_->supports(f);
    }

    bool requires(const Requirements& r) const override
    {
        return impl_->requires(r);
    }

    bool matches_criteria(const Criteria& criteria) override
    {
        return impl_->matches_criteria(criteria);
    }

    void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state) override
    {
        impl_->on_wifi_and_cell_reporting_state_changed(state);
    }

    void on_reference_location_updated(const Update<Position>& position) override
    {
        impl_->on_reference_location_updated(position);
    }

    void on_reference_velocity_updated(const Update<Velocity>& velocity) override
    {
        impl_->on_reference_velocity_updated(velocity);
    }

    void on_reference_heading_updated(const Update<Heading>& heading) override
    {
        impl_->on_reference_heading_updated(heading);
    }

    void start_position_updates() override
    {
        state_ = State::active;
        impl_->state_controller()->start_position_updates();
    }

    void stop_position_updates() override
    {
        state_ = State::enabled;
        impl_->state_controller()->stop_position_updates();
    }

    void start_velocity_updates() override
    {
        state_ = State::active;
        impl_->state_controller()->start_velocity_updates();
    }

    void stop_velocity_updates() override
    {
        state_ = State::enabled;
        impl_->state_controller()->stop_velocity_updates();
    }

    void start_heading_updates() override
    {
        state_ = State::active;
        impl_->state_controller()->start_heading_updates();
    }

    void stop_heading_updates() override
    {
        state_ = State::enabled;
        impl_->state_controller()->stop_heading_updates();
    }

private:
    Provider::Ptr impl_;
    struct
    {
        core::ScopedConnection position_updates;
        core::ScopedConnection heading_updates;
        core::ScopedConnection velocity_updates;
    } connections;
    core::Property<State> state_;
};
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_STATE_TRACKING_PROVIDER_H_
