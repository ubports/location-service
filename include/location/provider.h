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
#ifndef LOCATION_PROVIDER_H_
#define LOCATION_PROVIDER_H_

#include <location/configuration.h>
#include <location/criteria.h>
#include <location/event.h>
#include <location/position.h>
#include <location/space_vehicle.h>
#include <location/update.h>
#include <location/visibility.h>
#include <location/wifi_and_cell_reporting_state.h>

#include <core/property.h>
#include <core/signal.h>

#include <atomic>
#include <bitset>
#include <memory>

namespace location
{
/// @brief The Provider class is the abstract base of all positioning providers.
class LOCATION_DLL_PUBLIC Provider : public Event::Receiver
{
public:
    typedef std::shared_ptr<Provider> Ptr;    

    /// @brief Enumerates the requirements of a provider implementation.
    enum class Requirements : std::size_t
    {
        none = 0,                   ///< The provider does not require anything.
        satellites = 1 << 0,        ///< The provider requires satellites to be visible.
        cell_network = 1 << 1,      ///< The provider requires a cell-network to work correctly.
        data_network = 1 << 2,      ///< The provider requires a data-network to work correctly.
        monetary_spending = 1 << 3  ///< Using the provider results in monetary cost.
    };

    /// @cond
    virtual ~Provider() = default;

    Provider(const Provider&) = delete;
    Provider(Provider&&) = delete;
    Provider& operator=(const Provider&) = delete;
    Provider& operator=(Provider&&) = delete;
    /// @endcond

    /// @brief enable enables the provider, throws in case of issues.
    virtual void enable() = 0;

    /// @brief start disables the provider, throws in case of issues.
    virtual void disable() = 0;

    /// @brief activate triggers a state transition from enabled to active.
    virtual void activate() = 0;

    /// @brief deactivate triggers a state transition from active to enabled.
    virtual void deactivate() = 0;

    /// @brief requirements returns the requirements of the provider.
    virtual Requirements requirements() const = 0;

    /// @brief Checks if a provider satisfies criteria.
    /// @param [in] criteria The criteria to check.
    /// @return true iff the provider satisfies the given criteria.
    virtual bool satisfies(const Criteria& criteria) = 0;

    /// @brief position_updates returns a signal delivering position updates.
    virtual const core::Signal<Update<Position>>& position_updates() const = 0;

    /// @brief heading_updates returns a signal delivering heading updates.
    virtual const core::Signal<Update<units::Degrees>>& heading_updates() const = 0;

    /// @brief velocity_updates returns a signal delivering velocity updates.
    virtual const core::Signal<Update<units::MetersPerSecond>>& velocity_updates() const = 0;

protected:
    Provider() = default;
};

/// @brief operator| returns the bitwise or of lhs and rhs.
LOCATION_DLL_PUBLIC Provider::Requirements operator|(Provider::Requirements lhs, Provider::Requirements rhs);
/// @brief operator& returns the bitwise and of lhs and rhs.
LOCATION_DLL_PUBLIC Provider::Requirements operator&(Provider::Requirements lhs, Provider::Requirements rhs);
}

#endif // LOCATION_PROVIDER_H_
