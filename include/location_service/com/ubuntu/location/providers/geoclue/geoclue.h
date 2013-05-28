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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GEOCLUE_GEOCLUE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GEOCLUE_GEOCLUE_H_

#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/traits/service.h"
#include "org/freedesktop/dbus/types/struct.h"
#include "org/freedesktop/dbus/types/stl/tuple.h"

#include <string>

namespace dbus = org::freedesktop::dbus;

namespace org
{
namespace freedesktop
{
struct Geoclue
{
    enum class Status : int
    {
        error,
        unavailable,
        acquiring,
        available
    };

    friend std::ostream& operator<<(std::ostream& out, const Status& status)
    {
        static std::map<Status, std::string> lut = 
		{
                    {Status::error, "error"},
                    {Status::unavailable, "unavailable"},
                    {Status::acquiring, "acquiring"},
                    {Status::available, "available"}
		};

        return out << lut[status];
    }

    struct GetProviderInfo
    {
        inline static std::string name()
        {
            return "GetProviderInfo";
        } 
        typedef Geoclue Interface;
        typedef std::tuple<std::string, std::string> ResultType;
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct GetStatus
    {
        inline static std::string name()
        {
            return "GetStatus";
        } 
        typedef Geoclue Interface;
        typedef int32_t ResultType;
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct AddReference
    {
        inline static std::string name()
        {
            return "AddReference";
        } 
        typedef Geoclue Interface;
        typedef void ResultType;
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct RemoveReference
    {
        inline static std::string name()
        {
            return "RemoveReference";
        } 
        typedef Geoclue Interface;
        typedef void ResultType;
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct Address
    {
        struct GetAddress
        {
            inline static std::string name()
            {
                return "GetAddress";
            } 
            typedef Address Interface;
            typedef std::tuple<int32_t, std::map<std::string, std::string>, dbus::types::Struct<std::tuple<int32_t, double, double>>> ResultType;
            inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };

        struct Signals
        {
            struct AddressChanged
            {
                inline static std::string name()
                {
                    return "PositionChanged";
                };
                typedef Address Interface;
                typedef std::tuple<int32_t, std::map<std::string, std::string>, dbus::types::Struct<std::tuple<int32_t, double, double>>> ArgumentType;
            };
        };
    };

    struct Position
    {
    	struct Field
    	{
            Field() = delete;

            static const int none = 0;
            static const int latitude = 1;
            static const int longitude = 2;
            static const int altitude = 3;
    	};

    	typedef std::bitset<4> FieldFlags;

        struct GetPosition
        {
            inline static std::string name()
            {
                return "GetPosition";
            } 
            typedef Position Interface;
            typedef std::tuple<int32_t, int32_t, double, double, double, dbus::types::Struct<std::tuple<int32_t, double, double>>> ResultType;
            inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };

        struct Signals
        {
            struct PositionChanged
            {
                inline static std::string name()
                {
                    return "PositionChanged";
                };
                typedef Position Interface;
                typedef std::tuple<int32_t, int32_t, double, double, double, dbus::types::Struct<std::tuple<int32_t, double, double>>> ArgumentType;
            };
        };
    };

    struct Velocity
    {
    	struct Field
    	{
            Field() = delete;

            static const int none = 0;
            static const int speed = 1;
            static const int direction = 2;
            static const int climb = 3;
    	};

    	typedef std::bitset<4> FieldFlags;

        struct GetVelocity
        {
            inline static std::string name()
            {
                return "GetVelocity";
            } 
            typedef Velocity Interface;
            typedef std::tuple<int32_t, int32_t, double, double, double> ResultType;
            inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };
        struct Signals
        {
            struct VelocityChanged
            {
                inline static std::string name()
                {
                    return "VelocityChanged";
                };
                typedef Velocity Interface;
                typedef std::tuple<int32_t, int32_t, double, double, double> ArgumentType;
            };
        };
    };
};
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
struct Service<org::freedesktop::Geoclue>
{
    inline static const std::string& interface_name()
    {
        static const std::string s{"org.freedesktop.Geoclue"};
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Address>
{
    inline static const std::string& interface_name()
    {
        static const std::string s{"org.freedesktop.Geoclue.Address"};
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Position>
{
    inline static const std::string& interface_name()
    {
        static const std::string s{"org.freedesktop.Geoclue.Position"};
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Velocity>
{
    inline static const std::string& interface_name()
    {
        static const std::string s{"org.freedesktop.Geoclue.Velocity"};
        return s;
    }
};
}
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GEOCLUE_GEOCLUE_H_
