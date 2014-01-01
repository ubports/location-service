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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_

#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_enumerator.h>
#include <com/ubuntu/location/provider_selection.h>
#include <com/ubuntu/location/satellite_based_positioning_state.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/wifi_and_cell_reporting_state.h>

#include <core/property.h>

#include <mutex>
#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria;
class ProviderSelectionPolicy;
/**
 * @brief The Engine class encapsulates a positioning engine, relying on a set
 * of providers and reporters to acquire and publish location information.
 */
class Engine : public ProviderEnumerator
{
public:
    typedef std::shared_ptr<Engine> Ptr;

    /**
     * @brief The State enum models the current state of the engine
     */
    enum class Status
    {
        off, ///< The engine is currently offline.
        on, ///< The engine and providers are powered on but not navigating.
        active ///< The engine and providers are powered on and navigating.
    };

    /**
     * @brief The Configuration struct summarizes the state of the engine.
     */
    struct Configuration
    {
        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<SatelliteBasedPositioningState> satellite_based_positioning_state
        {
            SatelliteBasedPositioningState::on
        };
        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<WifiAndCellIdReportingState> wifi_and_cell_id_reporting_state
        {
            WifiAndCellIdReportingState::off
        };
        /** Setable/getable/observable property for the overall engine state. */
        core::Property<Engine::Status> engine_state
        {
            Engine::Status::on
        };
    };

    struct Updates
    {
        /** The current best known reference location */
        core::Property<Update<Position>> reference_location{};
        /** The current best known velocity estimate. */
        core::Property<Update<Velocity>> reference_velocity{};
        /** The current best known heading estimate. */
        core::Property<Update<Heading>> reference_heading{};
        /** The current set of visible SpaceVehicles. */
        core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>> visible_space_vehicles{};
    };

    Engine(const std::shared_ptr<ProviderSelectionPolicy>& provider_selection_policy);
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    /**
     * @brief Calculates a set of providers that satisfies the given criteria.
     * @param [in] criteria The criteria to be satisfied by the returned provider selection.
     * @return A provider selection that satisfies the given criteria.
     */
    virtual ProviderSelection determine_provider_selection_for_criteria(const Criteria& criteria);

    /**
     * @brief Checks if the engine knows about a specific provider.
     * @return True iff the engine knows about the provider.
     */
    virtual bool has_provider(const Provider::Ptr& provider) noexcept;

    /**
     * @brief Makes a provider known to the engine.
     * @param provider The new provider.
     */
    virtual void add_provider(const Provider::Ptr& provider);

    /**
     * @brief Removes a provider from the engine.
     * @param provider The provider to be removed.
     */
    virtual void remove_provider(const Provider::Ptr& provider) noexcept;

    /**
     * @brief Iterates all known providers and invokes the provided enumerator for each of them.
     * @param enumerator The functor to be invoked for each provider.
     */
    virtual void for_each_provider(const std::function<void(const Provider::Ptr&)>& enumerator) const noexcept;

    /** The engine's configuration. */
    Configuration configuration;

    /** All updates distributed via the engine. */
    Updates updates;

private:
    struct ProviderConnections
    {
        core::ScopedConnection reference_location_updates;
        core::ScopedConnection reference_velocity_updates;
        core::ScopedConnection reference_heading_updates;
        core::ScopedConnection wifi_and_cell_id_reporting_state_updates;
        core::ScopedConnection space_vehicle_visibility_updates;
    };

    mutable std::mutex guard;
    std::map<Provider::Ptr, ProviderConnections> providers;
    std::shared_ptr<ProviderSelectionPolicy> provider_selection_policy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
