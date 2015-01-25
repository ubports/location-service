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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_

#include <com/ubuntu/location/criteria.h>
#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/wifi_and_cell_reporting_state.h>

#include <core/property.h>
#include <core/signal.h>

#include <atomic>
#include <bitset>
#include <memory>

namespace com
{
namespace ubuntu
{
namespace location
{
/**
 * @brief The Provider class is the abstract base of all positioning providers.
 */
class Provider
{
public:
    typedef std::shared_ptr<Provider> Ptr;

    /**
     * @brief Enumerates the known features that can be supported by providers.
     */
    enum class Features : std::size_t
    {
        none = 0, ///< The provider does not support any feature.
        position = 1 << 0, ///< The provider features position updates.
        velocity = 1 << 1, ///< The provider features velocity updates.
        heading = 1 << 2 ///< The provider features heading updates.
    };    

    /**
     * @brief Enumerates the requirements of a provider implementation.
     */
    enum class Requirements : std::size_t
    {
        none = 0, ///< The provider does not require anything.
        satellites = 1 << 0, ///< The provider requires satellites to be visible.
        cell_network = 1 << 1, ///< The provider requires a cell-network to work correctly.
        data_network = 1 << 2, ///< The provider requires a data-network to work correctly.
        monetary_spending = 1 << 3 ///< Using the provider results in monetary cost.
    };

    /**
     * @brief Facade for controlling the state of position/heading/velocity updates.
     *
     * Multiple observers can request state changes for updates. This class ensures
     * that the specific updates are started and stopped if at least one observer
     * requests them and stopped when the last observer issues a stop request.
     */
    class Controller
    {
    public:
        typedef std::shared_ptr<Controller> Ptr; 

        virtual ~Controller() = default;
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;        

        /**
         * @brief disable switches the provider to a disabled state, such that subsequent
         * calls to start* methods fail.
         */
        void disable();

        /**
         * @brief enable switches the provider to an enabled state, such that subsequent
         * calls to start* methods succeed.
         */
        void enable();

        /**
         * @brief Request to start position updates if not already running.
         */
        virtual void start_position_updates();

        /**
         * @brief Request to stop position updates. Only stops the provider when the last observer calls this function.
         */
        virtual void stop_position_updates();

        /**
         * @brief Checks if position updates are currently running.
         * @return true iff position updates are currently running.
         */
        bool are_position_updates_running() const;

        /**
         * @brief Request to start heading updates if not already running.
         */
        virtual void start_heading_updates();

        /**
         * @brief Request to stop heading updates. Only stops the provider when the last observer calls this function.
         */
        virtual void stop_heading_updates();

        /**
         * @brief Checks if position updates are currently running.
         * @return true iff position updates are currently running.
         */
        bool are_heading_updates_running() const;

        /**
         * @brief Request to start velocity updates if not already running.
         */
        virtual void start_velocity_updates();

        /**
         * @brief Request to stop velocity updates. Only stops the provider when the last observer calls this function.
         */
        virtual void stop_velocity_updates();

        /**
         * @brief Checks if velocity updates are currently running.
         * @return true iff velocity updates are currently running.
         */
        bool are_velocity_updates_running() const;

    protected:
        friend class Provider;
        explicit Controller(Provider& instance);

    private:
        Provider& instance;
        std::atomic<int> position_updates_counter;
        std::atomic<int> heading_updates_counter;
        std::atomic<int> velocity_updates_counter;
    };

    /**
     * @brief Wraps all updates that can be delivered by a provider.
     */
    struct Updates
    {
        /** Position updates. */
        core::Signal<Update<Position>> position;
        /** Heading updates. */
        core::Signal<Update<Heading>> heading;
        /** Velocity updates. */
        core::Signal<Update<Velocity>> velocity;
        /** Space vehicle visibility updates. */
        core::Signal<Update<std::set<SpaceVehicle>>> svs;
    };

    virtual ~Provider() = default;

    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;

    /**
     * @brief Provides non-mutable access to this provider's updates.
     * @return A non-mutable reference to the updates.
     */
    virtual const Updates& updates() const;

    /**
     * @brief Access to the controller facade of this provider instance.
     */
    virtual const Controller::Ptr& state_controller() const;

    /**
     * @brief Checks if the provider supports a specific feature.
     * @param f Feature to test for
     * @return true iff the provider supports the feature.
     */
    virtual bool supports(const Features& f) const;

    /**
     * @brief Checks if the provider has got a specific requirement.
     * @param r Requirement to test for.
     * @return true iff the provider has the specific requirement.
     */
    virtual bool requires(const Requirements& r) const;

    /**
     * @brief Checks if a provider satisfies a set of accuracy criteria.
     * @param [in] criteria The criteria to check.
     * @return true iff the provider satisfies the given criteria.
     */
    virtual bool matches_criteria(const Criteria& criteria);

    /**
     * @brief Called by the engine whenever the wifi and cell ID reporting state changes.
     * @param state The new state.
     */
    virtual void on_wifi_and_cell_reporting_state_changed(WifiAndCellIdReportingState state);

    /**
     * @brief Called by the engine whenever the reference location changed.
     * @param position The new reference location.
     */
    virtual void on_reference_location_updated(const Update<Position>& position);

    /**
     * @brief Called by the engine whenever the reference velocity changed.
     * @param velocity The new reference velocity.
     */
    virtual void on_reference_velocity_updated(const Update<Velocity>& velocity);

    /**
     * @brief Called by the engine whenever the reference heading changed.
     * @param heading The new reference heading.
     */
    virtual void on_reference_heading_updated(const Update<Heading>& heading);

protected:
    explicit Provider(
        const Features& features = Features::none,
        const Requirements& requirements = Requirements::none);

    virtual Updates& mutable_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void start_position_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void stop_position_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void start_heading_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void stop_heading_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void start_velocity_updates();

    /**
     * @brief Implementation-specific, empty by default.
     */
    virtual void stop_velocity_updates();

private:
    struct
    {
        Features features = Features::none;
        Requirements requirements = Requirements::none;
        Updates updates;
        Controller::Ptr controller = Controller::Ptr{};
    } d;
};

Provider::Features operator|(Provider::Features lhs, Provider::Features rhs);
Provider::Features operator&(Provider::Features lhs, Provider::Features rhs);

Provider::Requirements operator|(Provider::Requirements lhs, Provider::Requirements rhs);
Provider::Requirements operator&(Provider::Requirements lhs, Provider::Requirements rhs);
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDER_H_
