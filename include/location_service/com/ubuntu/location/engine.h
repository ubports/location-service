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

#include "com/ubuntu/connectivity/cell.h"
#include "com/ubuntu/connectivity/wifi/network.h"

#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_selection_policy.h"
#include "com/ubuntu/location/units/units.h"

#include <com/ubuntu/property.h>

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
        Position position; ///< The actual location.
        units::Length accuracy; ///< The accuracy of the fix.
        std::set<com::ubuntu::connectivity::Cell> visible_cells; ///< The set of radio cells that were visible.
        std::set<com::ubuntu::connectivity::wifi::Network> visible_wireless_networks; ///< The wifi's that were visible.
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
        /**
         * @brief The SatelliteBasedPositioningState enum describes whether satellite assisted positioning is enabled or disabled.
         */
        enum class SatelliteBasedPositioningState
        {
            on, ///< Satellite assisted positioning is on.
            off ///< Satellite assisted positioning is off.
        };

        /** Setable/getable/observable property for the satellite based positioning state. */
        com::ubuntu::Property<SatelliteBasedPositioningState> satellite_based_positioning_state;
        /** Setable/getable/observable property for the overall engine state. */
        com::ubuntu::Property<Engine::Status> engine_state;
        /** The current best known reference location */
        com::ubuntu::Property<Update<LastKnownReferenceLocation>> reference_location;
    };

    Engine(const std::set<Provider::Ptr>& initial_providers,
           const ProviderSelectionPolicy::Ptr& provider_selection_policy);
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    virtual ProviderSelection determine_provider_selection_for_criteria(const Criteria& criteria);

    virtual bool has_provider(const Provider::Ptr& provider) noexcept;
    virtual void add_provider(const Provider::Ptr& provider);
    virtual void remove_provider(const Provider::Ptr& provider) noexcept;

private:
    std::set<Provider::Ptr> providers;
    ProviderSelectionPolicy::Ptr provider_selection_policy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_ENGINE_H_
