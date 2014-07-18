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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_P_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_P_H_

#include <com/ubuntu/location/service/session/interface.h>

#include <core/dbus/codec.h>
#include <core/dbus/traits/service.h>

struct com::ubuntu::location::service::session::Interface::UpdatePosition
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

struct com::ubuntu::location::service::session::Interface::UpdateVelocity
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

struct com::ubuntu::location::service::session::Interface::UpdateHeading
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

struct com::ubuntu::location::service::session::Interface::StartPositionUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::StopPositionUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::StartVelocityUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::StopVelocityUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::StartHeadingUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::StopHeadingUpdates
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

    inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{5}; }
};

struct com::ubuntu::location::service::session::Interface::Errors::ErrorParsingUpdate
{
    inline static std::string name()
    {
        return "com.ubuntu.location.Service.Session.ErrorParsingUpdate";
    }
};

struct com::ubuntu::location::service::session::Interface::Errors::ErrorStartingUpdate
{
    inline static std::string name()
    {
        return "com.ubuntu.location.Service.Session.ErrorStartingUpdate";
    }
};

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

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_SESSION_INTERFACE_P_H_
