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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CODEC_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CODEC_H_

#include "com/ubuntu/location/accuracy.h"
#include "com/ubuntu/location/criteria.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/position.h"
#include "com/ubuntu/location/update.h"
#include "com/ubuntu/location/velocity.h"
#include "com/ubuntu/location/units/units.h"
#include "com/ubuntu/location/wgs84/altitude.h"
#include "com/ubuntu/location/wgs84/latitude.h"
#include "com/ubuntu/location/wgs84/longitude.h"

#include <core/dbus/codec.h>

namespace core
{
namespace dbus
{
namespace helper
{
template<typename T>
struct TypeMapper<com::ubuntu::location::units::Quantity<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::floating_point;
    }
    constexpr static bool is_basic_type()
    {
        return true;
    }
    constexpr static bool requires_signature()
    {
        return false;
    }

    static std::string signature()
    {
        static const std::string s = TypeMapper<double>::signature();
        return s;
    }
};
}

template<typename T>
struct Codec<com::ubuntu::location::units::Quantity<T>>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::units::Quantity<T>& in)
    {
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::encode_argument(out, in.value());
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::units::Quantity<T>& in)
    {
        typename com::ubuntu::location::units::Quantity<T>::value_type value;
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::decode_argument(out, value);
        in = com::ubuntu::location::units::Quantity<T>::from_value(value);
    }    
};

namespace helper
{
template<typename T, typename U>
struct TypeMapper<com::ubuntu::location::wgs84::Coordinate<T,U>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                TypeMapper<com::ubuntu::location::units::Quantity<U>>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<typename T, typename U>
struct Codec<com::ubuntu::location::wgs84::Coordinate<T,U>>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::encode_argument(out, in.value);
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::decode_argument(out, in.value);
    }    
};

namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::Position>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
                TypeMapper<uint64_t>::signature() + 
                TypeMapper<com::ubuntu::location::wgs84::Latitude>::signature() +
                TypeMapper<com::ubuntu::location::wgs84::Longitude>::signature() +
                TypeMapper<com::ubuntu::location::wgs84::Altitude>::signature();
        return s;
    }
};
}

template<>
struct Codec<com::ubuntu::location::Position>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Position& in)
    {
        Codec<uint64_t>::encode_argument(out, in.flags().to_ulong());
        if (in.has_latitude())
            Codec<com::ubuntu::location::wgs84::Latitude>::encode_argument(out, in.latitude());
        if (in.has_longitude())
            Codec<com::ubuntu::location::wgs84::Longitude>::encode_argument(out, in.longitude());
        if (in.has_altitude())
            Codec<com::ubuntu::location::wgs84::Altitude>::encode_argument(out, in.altitude());
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Position& in)
    {
        com::ubuntu::location::wgs84::Latitude lat;
        com::ubuntu::location::wgs84::Longitude lon;
        com::ubuntu::location::wgs84::Altitude alt;
        uint64_t flags_on_wire;
        Codec<uint64_t>::decode_argument(out, flags_on_wire);

        com::ubuntu::location::Position::Flags flags{flags_on_wire};
        if (flags.test(com::ubuntu::location::Position::latitude_flag))
        {
            Codec<com::ubuntu::location::wgs84::Latitude>::decode_argument(out, lat);
            in.latitude(lat);
        }
        if (flags.test(com::ubuntu::location::Position::latitude_flag))
        {
            Codec<com::ubuntu::location::wgs84::Longitude>::decode_argument(out, lon);
            in.longitude(lon);
        }
        if (flags.test(com::ubuntu::location::Position::altitude_flag))
        {       
            Codec<com::ubuntu::location::wgs84::Altitude>::decode_argument(out, alt);
            in.altitude(alt);
        }
    }
};

namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::Velocity>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                TypeMapper<typename com::ubuntu::location::Velocity::Quantity>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<com::ubuntu::location::Velocity>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Velocity& in)
    {
        Codec<typename com::ubuntu::location::Velocity::Quantity>::encode_argument(out, in.value);
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Velocity& in)
    {
        Codec<typename com::ubuntu::location::Velocity::Quantity>::decode_argument(out, in.value);
    }
};

namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::Heading>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                TypeMapper<typename com::ubuntu::location::Heading::Quantity>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<com::ubuntu::location::Heading>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Heading& in)
    {
        Codec<typename com::ubuntu::location::Heading::Quantity>::encode_argument(out, in.value);
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Heading& in)
    {
        Codec<typename com::ubuntu::location::Heading::Quantity>::decode_argument(out, in.value);
    }
};

namespace helper
{
template<typename T>
struct TypeMapper<com::ubuntu::location::Accuracy<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
            TypeMapper<T>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<typename T>
struct Codec<com::ubuntu::location::Accuracy<T>>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Accuracy<T>& in)
    {
        Codec<T>::encode_argument(out, in.value);
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Accuracy<T>& in)
    {
        Codec<T>::decode_argument(out, in.value);
    }    
};

namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::Criteria>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                helper::TypeMapper<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Latitude>>::signature() +
                helper::TypeMapper<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Longitude>>::signature() +
                helper::TypeMapper<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Altitude>>::signature() +
                helper::TypeMapper<com::ubuntu::location::Accuracy<com::ubuntu::location::Velocity>>::signature() +
                helper::TypeMapper<com::ubuntu::location::Accuracy<com::ubuntu::location::Heading>>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<com::ubuntu::location::Criteria>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Criteria& in)
    {
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Latitude>>::encode_argument(out, in.latitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Longitude>>::encode_argument(out, in.longitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Altitude>>::encode_argument(out, in.altitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::Velocity>>::encode_argument(out, in.velocity_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::Heading>>::encode_argument(out, in.heading_accuracy);
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Criteria& in)
    {
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Latitude>>::decode_argument(out, in.latitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Longitude>>::decode_argument(out, in.longitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::wgs84::Altitude>>::decode_argument(out, in.altitude_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::Velocity>>::decode_argument(out, in.velocity_accuracy);
        Codec<com::ubuntu::location::Accuracy<com::ubuntu::location::Heading>>::decode_argument(out, in.heading_accuracy);
    }    
};
namespace helper
{
template<typename T>
struct TypeMapper<com::ubuntu::location::Update<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s =
                helper::TypeMapper<T>::signature() +
                helper::TypeMapper<uint64_t>::signature();
        return s;
    }
};
}

template<typename T>
struct Codec<com::ubuntu::location::Update<T>>
{
    static void encode_argument(core::dbus::Message::Writer& out, const com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::encode_argument(out, in.value);
        Codec<int64_t>::encode_argument(out, in.when.time_since_epoch().count());
    }

    static void decode_argument(core::dbus::Message::Reader& out, com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::decode_argument(out, in.value);
        int64_t value;
        Codec<int64_t>::decode_argument(out, value);
        in.when = com::ubuntu::location::Clock::Timestamp(com::ubuntu::location::Clock::Duration(value));
    }    
};
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CODEC_H_
