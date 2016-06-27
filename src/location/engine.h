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
#ifndef LOCATION_ENGINE_H_
#define LOCATION_ENGINE_H_

#include <location/provider.h>
#include <location/provider_enumerator.h>
#include <location/provider_selection.h>
#include <location/provider_selection_policy.h>
#include <location/satellite_based_positioning_state.h>
#include <location/space_vehicle.h>
#include <location/state_tracking_provider.h>
#include <location/wifi_and_cell_reporting_state.h>

#include <location/settings.h>

#include <core/property.h>

#include <mutex>
#include <set>

#include "update_policy.h"

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
        /** Keys for persisting state go here. */
        struct Keys
        {
            /** Key for persisting the SatelliteBasedPositioningState */
            static constexpr const char* satellite_based_positioning_state
            {
                "Engine::SatelliteBasedPositioningState"
            };
            /** Key for persisting the WifiAndCellIdReportingState */
            static constexpr const char* wifi_and_cell_id_reporting_state
            {
                "Engine::WifiAndCellIdReportingState"
            };
            /** Key for persisting the Engine::Status */
            static constexpr const char* engine_state
            {
                "Engine::State"
            };
        };

        /** Default values go here. */
        struct Defaults
        {
            static constexpr const SatelliteBasedPositioningState satellite_based_positioning_state
            {
                SatelliteBasedPositioningState::on
            };

            static constexpr const WifiAndCellIdReportingState wifi_and_cell_id_reporting_state
            {
                WifiAndCellIdReportingState::off
            };

            static constexpr const Engine::Status engine_state
            {
                Engine::Status::on
            };
        };

        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<SatelliteBasedPositioningState> satellite_based_positioning_state
        {
            Defaults::satellite_based_positioning_state
        };
        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<WifiAndCellIdReportingState> wifi_and_cell_id_reporting_state
        {
            Defaults::wifi_and_cell_id_reporting_state
        };
        /** Setable/getable/observable property for the overall engine state. */
        core::Property<Engine::Status> engine_state
        {
            Defaults::engine_state
        };
    };

    /** @brief Summarizes all updates delivered via the engine. */
    struct Updates
    {
        /** The current best known reference location */
        core::Property<Optional<Update<Position>>> last_known_location{};
        /** The current best known velocity estimate. */
        core::Property<Optional<Update<Velocity>>> last_known_velocity{};
        /** The current best known heading estimate. */
        core::Property<Optional<Update<Heading>>> last_known_heading{};
        /** The current set of visible SpaceVehicles. */
        core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>> visible_space_vehicles{};
    };

    Engine(const ProviderSelectionPolicy::Ptr& provider_selection_policy,
           const Settings::Ptr& settings);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine();

    /**
     * @brief Calculates a set of providers that satisfies the given criteria.
     * @param [in] criteria The criteria to be satisfied by the returned provider selection.
     * @return A provider selection that satisfies the given criteria.
     */
    virtual ProviderSelection determine_provider_selection_for_criteria(const Criteria& criteria);

    /**
     * @brief Makes a provider known to the engine.
     * @param provider The new provider.
     */
    virtual void add_provider(const Provider::Ptr& provider);

    /**
     * @brief Iterates all known providers and invokes the provided enumerator for each of them.
     * @param enumerator The functor to be invoked for each provider.
     */
    virtual void for_each_provider(const std::function<void(const Provider::Ptr&)>& enumerator) const noexcept;

    /** @brief The engine's configuration. */
    Configuration configuration;

    /** @brief All updates distributed via the engine. */
    Updates updates;

private:
    struct ProviderConnections
    {
        core::ScopedConnection reference_location_updates;
        core::ScopedConnection reference_velocity_updates;
        core::ScopedConnection reference_heading_updates;
        core::ScopedConnection wifi_and_cell_id_reporting_state_updates;
        core::ScopedConnection space_vehicle_visibility_updates;
        core::ScopedConnection provider_position_updates;
        core::ScopedConnection provider_state_updates;
    };

    mutable std::recursive_mutex guard;
    std::map<StateTrackingProvider::Ptr, ProviderConnections> providers;
    ProviderSelectionPolicy::Ptr provider_selection_policy;
    Settings::Ptr settings;
    UpdatePolicy::Ptr update_policy;
};

/** @brief Pretty prints the given status to the given stream. */
std::ostream& operator<<(std::ostream&, Engine::Status);
/** @brief Parses the status from the given stream. */
std::istream& operator>>(std::istream&, Engine::Status&);
}

#endif // LOCATION_ENGINE_H_
