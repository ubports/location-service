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

#include <location/providers/state_tracking_provider.h>

void location::providers::StateTrackingProvider::ReferenceCountedOnce::increment(const std::function<void()>& task)
{
    if (counter++ == 0) task();
}

void location::providers::StateTrackingProvider::ReferenceCountedOnce::decrement(const std::function<void()>& task)
{
    if (counter == 0) return;
    if (counter-- == 1) task();
}

location::providers::StateTrackingProvider::InvalidStateTransition::InvalidStateTransition(State from, State to)
    : std::runtime_error{"Invalid state transition"}, from{from}, to{to}
{
}

location::providers::StateTrackingProvider::StateTrackingProvider(const Provider::Ptr& impl)
    : impl_{impl}, state_{State::enabled}
{
}

const core::Property<location::providers::StateTrackingProvider::State>& location::providers::StateTrackingProvider::state() const
{
    return state_;
}

void location::providers::StateTrackingProvider::on_new_event(const Event& ev)
{
    impl_->on_new_event(ev);
}

void location::providers::StateTrackingProvider::enable()
{
    std::lock_guard<std::recursive_mutex> lg{guard_};

    switch (state())
    {
    case State::disabled:
        state_ = State::enabled;
        break;
    case State::enabled:
        break;
    case State::active:
        break;
    }
}

void location::providers::StateTrackingProvider::disable()
{
    std::lock_guard<std::recursive_mutex> lg{guard_};

    switch (state())
    {
    case State::disabled:
        break;
    case State::enabled:
        impl_->disable(); state_ = State::disabled;
        break;
    case State::active:
        throw InvalidStateTransition{State::disabled, State::active};
    }
}

void location::providers::StateTrackingProvider::activate()
{
    std::lock_guard<std::recursive_mutex> lg{guard_};

    switch (state())
    {
    case State::disabled:
        throw InvalidStateTransition{State::disabled, State::active};
        break;
    case State::enabled:        
    case State::active:
        activation_counter_.increment([this]() { impl_->activate(); state_ = State::active; });
        break;
    }
}

void location::providers::StateTrackingProvider::deactivate()
{
    std::lock_guard<std::recursive_mutex> lg{guard_};

    switch (state())
    {
    case State::disabled:
        break;
    case State::enabled:
        break;
    case State::active:
        activation_counter_.decrement([this]() { impl_->deactivate(); state_ = State::enabled;});
        break;
    }
}

location::Provider::Requirements location::providers::StateTrackingProvider::requirements() const
{
    return impl_->requirements();
}

bool location::providers::StateTrackingProvider::satisfies(const Criteria& criteria)
{
    return impl_->satisfies(criteria);
}

const core::Signal<location::Update<location::Position>>& location::providers::StateTrackingProvider::position_updates() const
{
    return impl_->position_updates();
}

const core::Signal<location::Update<location::Heading>>& location::providers::StateTrackingProvider::heading_updates() const
{
    return impl_->heading_updates();
}

const core::Signal<location::Update<location::Velocity>>& location::providers::StateTrackingProvider::velocity_updates() const
{
    return impl_->velocity_updates();
}
