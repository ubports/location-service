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

#ifndef LOCATION_DBUS_SESSION_H_
#define LOCATION_DBUS_SESSION_H_

#include <chrono>
#include <string>

namespace location
{
namespace dbus
{
struct Session
{
    struct UpdatePosition
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"UpdatePosition"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct UpdateVelocity
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"UpdateVelocity"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct UpdateHeading
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"UpdateHeading"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct StartPositionUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StartPositionUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct StopPositionUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StopPositionUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct StartVelocityUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StartVelocityUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct StopVelocityUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StopVelocityUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct StartHeadingUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StartHeadingUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct StopHeadingUpdates
    {
        typedef dbus::Session Interface;

        inline static const std::string& name()
        {
            static const std::string s{"StopHeadingUpdates"};
            return s;
        }

        typedef void ResultType;

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
    };

    struct Errors
    {
        struct ErrorParsingUpdate
        {
            inline static std::string name(){return "com.ubuntu.location.Service.Session.ErrorParsingUpdate";}
        };

        struct ErrorStartingUpdate
        {
            inline static std::string name(){return "com.ubuntu.location.Service.Session.ErrorStartingUpdate";}
        };
    };
};
}
}

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<location::dbus::Session>
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

#endif // LOCATION_DBUS_SESSION_H_
