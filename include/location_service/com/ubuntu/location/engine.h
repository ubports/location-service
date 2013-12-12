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

#include <com/ubuntu/location/criteria.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/provider_selection_policy.h>
#include <com/ubuntu/location/reporter.h>
#include <com/ubuntu/location/satellite_based_positioning_state.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/wifi_and_cell_reporting_state.h>
#include <com/ubuntu/location/units/units.h>

#include <core/property.h>

#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
/**
 * @brief The Engine class encapsulates a positioning engine, relying on a set
 * of providers and reporters to acquire and publish location information.
 */
class Engine
{
public:
    typedef std::shared_ptr<Engine> Ptr;

    /**
     * @brief The LastKnownReferenceLocation struct contains the best, last known fix.
     */
    struct LastKnownReferenceLocation
    {        
        bool operator==(const LastKnownReferenceLocation&) const
        {
            return false;
        }

        Position position = Position(); ///< The actual location.
        units::Quantity<units::Length> accuracy = 1E10* units::Meters; ///< The accuracy of the fix.
    };

    /**
     * @brief The State enum models the current state of the engine
     */
    enum class Status
    {
        off, ///< The engine is currently offline.
        on, ///< The engine and providers are powered on but not navigating.
        active ///< The engine and providers are powerd on and navigating.
    };

    /**
     * @brief The Configuration struct summarizes the state of the engine.
     */
    struct Configuration
    {
        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<SatelliteBasedPositioningState> satellite_based_positioning_state;
        /** Setable/getable/observable property for the satellite based positioning state. */
        core::Property<WifiAndCellIdReportingState> wifi_and_cell_id_reporting_state;
        /** Setable/getable/observable property for the overall engine state. */
        core::Property<Engine::Status> engine_state;
        /** The current best known reference location */
        core::Property<Update<LastKnownReferenceLocation>> reference_location;
        /** The current set of visible SpaceVehicles. */
        core::Property<std::set<SpaceVehicle>> visible_space_vehicles;
    };

    Engine(const std::set<Provider::Ptr>& initial_providers,
           const ProviderSelectionPolicy::Ptr& provider_selection_policy);
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
     * @brief Checks if the engine knows about a specific reporter.
     * @return True iff the engine knows about the reporter.
     */
    virtual bool has_reporter(const Reporter::Ptr& reporter) noexcept;

    /**
     * @brief Makes a reporter known to the engine.
     * @param reporter The new reporter.
     */
    virtual void add_reporter(const Reporter::Ptr& reporter);

    /**
     * @brief Removes a reporter from the engine.
     * @param reporter The reporter to be removed.
     */
    virtual void remove_reporter(const Reporter::Ptr& reporter) noexcept;

    /** The engine's configuration. */
    Configuration configuration;

private:
    std::set<Provider::Ptr> providers;
    std::set<Reporter::Ptr> reporters;
    ProviderSelectionPolicy::Ptr provider_selection_policy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
