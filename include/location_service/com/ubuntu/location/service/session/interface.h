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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_H_

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>

#include <core/property.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
namespace session
{
/**
 * @brief Models a session with the location service.
 */
class Interface
{
public:
    struct UpdatePosition;
    struct UpdateVelocity;
    struct UpdateHeading;

    struct StartPositionUpdates;
    struct StopPositionUpdates;

    struct StartVelocityUpdates;
    struct StopVelocityUpdates;

    struct StartHeadingUpdates;
    struct StopHeadingUpdates;

    struct Errors
    {
        struct ErrorParsingUpdate;
        struct ErrorStartingUpdate;
    };
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

        /**
         * @brief Updates for position measurements.
         */
        core::Property<Update<Position>> position{};
        /**
         * @brief Status of position updates, mutable.
         */
        core::Property<Status> position_status{Status::disabled};

        /**
         * @brief Updates for the heading measurements.
         */
        core::Property<Update<Heading>> heading{};
        /**
         * @brief Status of position updates, mutable.
         */
        core::Property<Status> heading_status{Status::disabled};

        /**
         * @brief Updates for velocity measurements.
         */
        core::Property<Update<Velocity>> velocity{};
        /**
         * @brief Status of velocity updates, mutable.
         */
        core::Property<Status> velocity_status{Status::disabled};
    };

    typedef std::shared_ptr<Interface> Ptr;

    Interface(const Interface&) = delete;
    virtual ~Interface() noexcept;
    Interface& operator=(const Interface&) = delete;

    /**
     * @brief Provides access to the updates delivered for this session.
     * @return A mutable reference to updates.
     */
    virtual Updates& updates();

protected:    
    Interface();

private:
    struct Private;
    std::shared_ptr<Private> d;
};
}
}
}
}
}


#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_H_
