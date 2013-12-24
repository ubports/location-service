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

#include <com/ubuntu/location/channel.h>
#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>

#include <core/property.h>

#include <core/dbus/codec.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/object_path.h>

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
    struct UpdatePosition
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "UpdatePosition"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct UpdateVelocity
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "UpdateVelocity"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct UpdateHeading
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "UpdateHeading"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StartPositionUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StartPositionUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StopPositionUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StopPositionUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StartVelocityUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StartVelocityUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StopVelocityUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StopVelocityUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StartHeadingUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StartHeadingUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StopHeadingUpdates
    {
        typedef com::ubuntu::location::service::session::Interface Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "StopHeadingUpdates"
            };
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct Errors
    {
        struct ErrorParsingUpdate
        {
            inline static std::string name()
            {
                return "com.ubuntu.location.Service.Session.ErrorParsingUpdate";
            }
        };

        struct ErrorStartingUpdate
        {
            inline static std::string name()
            {
                return "com.ubuntu.location.Service.Session.ErrorStartingUpdate";
            }
        };
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
        core::Property<Update<Position>> position;
        /**
         * @brief Status of position updates, mutable.
         */
        core::Property<Status> position_status;

        /**
         * @brief Updates for the heading measurements.
         */
        core::Property<Update<Heading>> heading;
        /**
         * @brief Status of position updates, mutable.
         */
        core::Property<Status> heading_status;

        /**
         * @brief Updates for velocity measurements.
         */
        core::Property<Update<Velocity>> velocity;
        /**
         * @brief Status of velocity updates, mutable.
         */
        core::Property<Status> velocity_status;
    };

    /** Forward declaration for an ID uniquely identifying this session. */
    struct Id;

    typedef std::shared_ptr<Interface> Ptr;

    Interface(const Interface&) = delete;
    virtual ~Interface() noexcept;
    Interface& operator=(const Interface&) = delete;

    virtual const core::dbus::types::ObjectPath& path() const = 0;

    /**
     * @brief Provides access to the updates delivered for this session.
     * @return A mutable reference to updates.
     */
    virtual Updates& updates();

protected:
    Interface();

private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
}
namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<com::ubuntu::location::service::session::Interface>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            "com.ubuntu.location.Service.Session"
        };
        return s;
    }
};
}
}
}

#include <com/ubuntu/location/codec.h>

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_H_
