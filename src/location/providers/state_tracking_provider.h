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
#ifndef LOCATION_PROVIDERS_STATE_TRACKING_PROVIDER_H_
#define LOCATION_PROVIDERS_STATE_TRACKING_PROVIDER_H_

#include <location/provider.h>
#include <location/visibility.h>

#include <functional>
#include <mutex>

namespace location
{
namespace providers
{
/// @brief StateTrackingProvider implements a simple state-machine making sure
/// that only supported and valid state transitions are triggered. In addition,
/// an activation count is tracked that ensures that providers are only ever started/stopped once.
class LOCATION_DLL_PUBLIC StateTrackingProvider : public Provider
{
public:
    typedef std::shared_ptr<StateTrackingProvider> Ptr;

    /// @brief State models the different states of a provider.
    enum class State
    {
        disabled,   ///< The provider is disabled. Implementations are free to minimize resource usage in this state.
        enabled,    ///< The provider is actively delivering updates.
        active      ///< The provider is actively delivering updates.
    };

    struct InvalidStateTransition : public std::runtime_error
    {
        InvalidStateTransition(State from, State to);
        State from;
        State to;
    };

    StateTrackingProvider(const Provider::Ptr& impl);

    const core::Property<State>& state() const;

    // From Provider
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
    class ReferenceCountedOnce
    {
    public:
        // increment increments the internal counter, executing task if the new value is 1.
        void increment(const std::function<void()>& task);

        // decrement decrements the internal counter and executes task if 0 is reached.
        void decrement(const std::function<void()>& task);

    private:
        std::size_t counter{0};
    };

    Provider::Ptr impl_;
    std::recursive_mutex guard_;
    ReferenceCountedOnce activation_counter_;
    core::Property<State> state_;
};
}
}

#endif // LOCATION_PROVIDERS_STATE_TRACKING_PROVIDER_H_
