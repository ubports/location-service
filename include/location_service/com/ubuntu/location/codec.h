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

#include <com/ubuntu/location/criteria.h>
#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/units/units.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

#include <org/freedesktop/dbus/codec.h>

namespace org
{
namespace freedesktop
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
struct Codec<com::ubuntu::location::Optional<T>>
{
    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::Optional<T>& in)
    {
        bool has_value{in};
        Codec<bool>::encode_argument(out, has_value);
        if (has_value)
            Codec<typename com::ubuntu::location::Optional<T>::value_type>::encode_argument(out, *in);
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::Optional<T>& in)
    {
        bool has_value{false};
        Codec<bool>::decode_argument(out, has_value); dbus_message_iter_next(out);
        if (has_value)
        {
            typename com::ubuntu::location::Optional<T>::value_type value;
            Codec<typename com::ubuntu::location::Optional<T>::value_type>::decode_argument(out, value);
            in = value;
        } else
        {
            in.reset();
        }
    }
};

template<typename T>
struct Codec<com::ubuntu::location::units::Quantity<T>>
{
    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::units::Quantity<T>& in)
    {
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::encode_argument(out, in.value());
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::units::Quantity<T>& in)
    {
        typename com::ubuntu::location::units::Quantity<T>::value_type value;
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::decode_argument(out, value);
        in = com::ubuntu::location::units::Quantity<T>::from_value(value);
        dbus_message_iter_next(out);
    }    
};

template<typename T, typename U>
struct Codec<com::ubuntu::location::wgs84::Coordinate<T,U>>
{
    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::encode_argument(out, in.value);
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::decode_argument(out, in.value);
    }    
};

template<>
struct Codec<com::ubuntu::location::Position>
{
    typedef com::ubuntu::location::Position::Accuracy::Horizontal HorizontalAccuracy;
    typedef com::ubuntu::location::Position::Accuracy::Vertical VerticalAccuracy;

    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::Position& in)
    {
        Codec<com::ubuntu::location::wgs84::Latitude>::encode_argument(out, in.latitude);
        Codec<com::ubuntu::location::wgs84::Longitude>::encode_argument(out, in.longitude);
        Codec<com::ubuntu::location::Optional<com::ubuntu::location::wgs84::Altitude>>::encode_argument(out, in.altitude);

        Codec<com::ubuntu::location::Optional<HorizontalAccuracy>>::encode_argument(out, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::encode_argument(out, in.accuracy.vertical);
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::Position& in)
    {
        Codec<com::ubuntu::location::wgs84::Latitude>::decode_argument(out, in.latitude);
        Codec<com::ubuntu::location::wgs84::Longitude>::decode_argument(out, in.longitude);
        Codec<com::ubuntu::location::Optional<com::ubuntu::location::wgs84::Altitude>>::decode_argument(out, in.altitude);

        Codec<com::ubuntu::location::Optional<HorizontalAccuracy>>::decode_argument(out, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::decode_argument(out, in.accuracy.vertical);
    }
};

namespace helper
{
template<typename T>
struct TypeMapper<std::set<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::array;
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
        static const std::string s = DBUS_TYPE_ARRAY_AS_STRING + TypeMapper<typename std::decay<T>::type>::signature();
        return s;
    }
};
}
template<typename T>
struct Codec<std::set<T>>
{
    static void encode_argument(DBusMessageIter* out, const std::set<T>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_ARRAY,
                    helper::TypeMapper<T>::requires_signature() ? helper::TypeMapper<T>::signature().c_str() : NULL,
                    std::addressof(sub)))
            throw std::runtime_error("Problem opening container");

        std::for_each(
            arg.begin(),
            arg.end(),
            std::bind(Codec<T>::encode_argument, std::addressof(sub), std::placeholders::_1));

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    static void decode_argument(DBusMessageIter* in, std::set<T>& out)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::array))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::array");

        if (dbus_message_iter_get_element_type(in) != static_cast<int>(helper::TypeMapper<T>::type_value()))
            throw std::runtime_error("Incompatible element type");

        int current_type;
        DBusMessageIter sub;
        dbus_message_iter_recurse(in, std::addressof(sub));
        while ((current_type = dbus_message_iter_get_arg_type (std::addressof(sub))) != DBUS_TYPE_INVALID)
        {
            T value;
            Codec<T>::decode_argument(std::addressof(sub), value);
            out.insert(value);

            dbus_message_iter_next(std::addressof(sub));
        }
    }
};
namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::SpaceVehicle>
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
                helper::TypeMapper<std::uint32_t>::signature() +
                helper::TypeMapper<std::size_t>::signature() +
                helper::TypeMapper<float>::signature() +
                helper::TypeMapper<bool>::signature() +
                helper::TypeMapper<bool>::signature() +
                helper::TypeMapper<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::signature() +
                helper::TypeMapper<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}
template<>
struct Codec<com::ubuntu::location::SpaceVehicle>
{
    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::SpaceVehicle& in)
    {
        DBusMessageIter sub;
        dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_STRUCT,
                    nullptr,
                    std::addressof(sub));
        std::uint32_t value = static_cast<std::uint32_t>(in.type);
        Codec<std::uint32_t>::encode_argument(&sub, value);
        Codec<std::size_t>::encode_argument(&sub, in.id);
        Codec<float>::encode_argument(&sub, in.snr);
        Codec<bool>::encode_argument(&sub, in.has_almanac_data);
        Codec<bool>::encode_argument(&sub, in.has_ephimeris_data);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::encode_argument(&sub, in.azimuth);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::encode_argument(&sub, in.elevation);
        dbus_message_iter_close_container(out, &sub);
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::SpaceVehicle& in)
    {
        DBusMessageIter sub;
        dbus_message_iter_recurse(out, std::addressof(sub));
        std::uint32_t value;
        Codec<std::uint32_t>::decode_argument(&sub, value); dbus_message_iter_next(&sub);
        in.type = static_cast<com::ubuntu::location::SpaceVehicle::Type>(value);
        Codec<std::size_t>::decode_argument(&sub, in.id); dbus_message_iter_next(&sub);
        Codec<float>::decode_argument(&sub, in.snr); dbus_message_iter_next(&sub);
        Codec<bool>::decode_argument(&sub, in.has_almanac_data); dbus_message_iter_next(&sub);
        Codec<bool>::decode_argument(&sub, in.has_ephimeris_data); dbus_message_iter_next(&sub);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::decode_argument(&sub, in.azimuth);
        dbus_message_iter_next(&sub);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::decode_argument(&sub, in.elevation);
        dbus_message_iter_next(&sub);
    }
};

template<>
struct Codec<com::ubuntu::location::Criteria>
{
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Length> HorizontalAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Length> VerticalAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Velocity> VelocityAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle> HeadingAccuracy;

    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::Criteria& in)
    {
        Codec<bool>::encode_argument(out, in.requires.position);
        Codec<bool>::encode_argument(out, in.requires.altitude);
        Codec<bool>::encode_argument(out, in.requires.heading);
        Codec<bool>::encode_argument(out, in.requires.velocity);

        Codec<HorizontalAccuracy>::encode_argument(out, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::encode_argument(out, in.accuracy.vertical);
        Codec<com::ubuntu::location::Optional<VelocityAccuracy>>::encode_argument(out, in.accuracy.velocity);
        Codec<com::ubuntu::location::Optional<HeadingAccuracy>>::encode_argument(out, in.accuracy.heading);
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::Criteria& in)
    {
        Codec<bool>::decode_argument(out, in.requires.position); dbus_message_iter_next(out);
        Codec<bool>::decode_argument(out, in.requires.altitude); dbus_message_iter_next(out);
        Codec<bool>::decode_argument(out, in.requires.heading); dbus_message_iter_next(out);
        Codec<bool>::decode_argument(out, in.requires.velocity); dbus_message_iter_next(out);

        Codec<HorizontalAccuracy>::decode_argument(out, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::decode_argument(out, in.accuracy.vertical);
        Codec<com::ubuntu::location::Optional<VelocityAccuracy>>::decode_argument(out, in.accuracy.velocity);
        Codec<com::ubuntu::location::Optional<HeadingAccuracy>>::decode_argument(out, in.accuracy.heading);
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
    static void encode_argument(DBusMessageIter* out, const com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::encode_argument(out, in.value);
        Codec<int64_t>::encode_argument(out, in.when.time_since_epoch().count());
    }

    static void decode_argument(DBusMessageIter* out, com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::decode_argument(out, in.value);
        int64_t value;
        Codec<int64_t>::decode_argument(out, value);
        dbus_message_iter_next(out);
        in.when = com::ubuntu::location::Clock::Timestamp(com::ubuntu::location::Clock::Duration(value));
    }    
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CODEC_H_
