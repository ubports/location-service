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

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/traits/service.h>
#include <org/freedesktop/dbus/types/object_path.h>

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
        struct ErrorParsingUpdate { inline static std::string name() { return "com.ubuntu.location.Service.Session.ErrorParsingUpdate"; } };
        struct ErrorStartingUpdate { inline static std::string name() { return "com.ubuntu.location.Service.Session.ErrorStartingUpdate"; } };
    };

    typedef std::shared_ptr<Interface> Ptr;

    Interface(const Interface&) = delete;
    virtual ~Interface() noexcept;
    Interface& operator=(const Interface&) = delete;

    virtual const org::freedesktop::dbus::types::ObjectPath& path() const = 0;

    ChannelConnection install_position_updates_handler(std::function<void(const Update<Position>&)> handler);
    ChannelConnection install_velocity_updates_handler(std::function<void(const Update<Velocity>&)> handler);
    ChannelConnection install_heading_updates_handler(std::function<void(const Update<Heading>&)> handler);

    virtual void start_position_updates() = 0;
    virtual void stop_position_updates() noexcept = 0;
    virtual void start_velocity_updates() = 0;
    virtual void stop_velocity_updates() noexcept = 0;
    virtual void start_heading_updates() = 0;
    virtual void stop_heading_updates() noexcept = 0;

protected:
    Interface();

    Channel<Update<Position>>& access_position_updates_channel();
    Channel<Update<Heading>>& access_heading_updates_channel();
    Channel<Update<Velocity>>& access_velocity_updates_channel();

private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}
}
}
namespace org
{
namespace freedesktop
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
}

#include "com/ubuntu/location/codec.h"

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_H_