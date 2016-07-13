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
#ifndef LOCATION_SERVICE_H_
#define LOCATION_SERVICE_H_

#include <location/space_vehicle.h>
#include <location/heading.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/velocity.h>
#include <location/update.h>

#include <core/property.h>

#include <chrono>
#include <functional>

namespace location
{
struct Criteria;

/**
 * @brief The Interface class models the primary interface to the location service.
 */
class Service
{
public:
    // Safe us some typing.
    typedef std::shared_ptr<Service> Ptr;

    /**
     * @brief Models a session with the location service.
     */
    class Session
    {
    public:
        /**
         * @brief Encapsulates updates provided for this session, and the ability to enable/disable updates.
         */
        struct Updates
        {
            /**
             * @brief The Status enum models the possible states of updates.
             */
            enum class Status
            {
                enabled, ///< Updates are enabled and delivered to this session.
                disabled ///< Updates are disabled and not delivered to this session.
            };

            core::Property<Update<Position>> position{};                ///< @brief Updates for position measurements.
            core::Property<Status> position_status{Status::disabled};   ///< @brief Status of position updates, mutable.
            core::Property<Update<Heading>> heading{};                  ///< @brief Updates for the heading measurements.
            core::Property<Status> heading_status{Status::disabled};    ///< @brief Status of position updates, mutable.
            core::Property<Update<Velocity>> velocity{};                ///< @brief Updates for velocity measurements.
            core::Property<Status> velocity_status{Status::disabled};   ///< @brief Status of velocity updates, mutable.
        };

        // Safe us some typing.
        typedef std::shared_ptr<Session> Ptr;

        /** @cond */
        Session(const Session&) = delete;
        virtual ~Session() = default;
        Session& operator=(const Session&) = delete;
        /** @endcond */

        /**
         * @brief Provides access to the updates delivered for this session.
         * @return A mutable reference to updates.
         */
        virtual Updates& updates() = 0;

    protected:
        Session() = default;
    };

    /// @brief State enumerates the known states of the service.
    enum class State
    {
        disabled, ///< The service has been disabled by the user.
        enabled,  ///< The service is enabled but not actively carrying out positioning.
        active    ///< The service is actively trying to determine the position of the device.
    };

    /** @cond */
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    virtual ~Service() = default;
    /** @endcond */

    /**
      * @brief The overall state of the service.
      * @return a getable/observable property.
      */
    virtual const core::Property<State>& state() const = 0;

    /**
     * @brief Whether the service uses satellite-based positioning.
     * @return  A setable/getable/observable property.
     */
    virtual core::Property<bool>& does_satellite_based_positioning() = 0;

    /**
     * @brief Whether the overall service and its positioning engine is online or not.
     * @return  A setable/getable/observable property.
     */
    virtual core::Property<bool>& is_online() = 0;

    /**
     * @brief Whether the engine and its providers/reporters do call home to
     * report reference locations together with wifi and cell ids.
     *
     * We consider this feature privacy sensitive and it defaults to false. The
     * user has to explicitly opt-in into this feature.
     *
     * @return A setable/getable/observable property.
     */
    virtual core::Property<bool>& does_report_cell_and_wifi_ids() = 0;

    /**
      * @brief All space vehicles currently visible.
      */
    virtual core::Property<std::map<SpaceVehicle::Key, SpaceVehicle>>& visible_space_vehicles() = 0;

    /**
     * @brief add_provider installs provider to the underlying location::Engine.
     */
    virtual void add_provider(const Provider::Ptr& provider) = 0;

    /**
     * @brief Starts a new session for the given criteria
     * @throw std::runtime_error in case of errors.
     * @param criteria The client's requirements in terms of accuraccy and functionality
     * @return A session instance.
     */
    virtual Session::Ptr create_session_for_criteria(const Criteria& criteria) = 0;

protected:
    Service() = default;
};

/// @brief operator== returns true if lhs == rhs.
bool operator==(Service::State lhs, Service::State rhs);
/// @brief operator != returns true if !(lhs == rhs).
bool operator!=(Service::State lhs, Service::State rhs);

/// @brief operator<< inserts state into the out.
std::ostream& operator<<(std::ostream& out, Service::State state);
/// @brief operator>> extracts state from in.
std::istream& operator>>(std::istream& in, Service::State& state);

}

#endif // LOCATION_SERVICE_H_
