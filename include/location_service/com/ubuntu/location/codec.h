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
#include <com/ubuntu/location/provider.h>
#include <com/ubuntu/location/space_vehicle.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>
#include <com/ubuntu/location/units/units.h>
#include <com/ubuntu/location/wgs84/altitude.h>
#include <com/ubuntu/location/wgs84/latitude.h>
#include <com/ubuntu/location/wgs84/longitude.h>

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
struct Codec<com::ubuntu::location::Optional<T>>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Optional<T>& in)
    {
        bool has_value{in};
        Codec<bool>::encode_argument(writer, has_value);
        if (has_value)
            Codec<typename com::ubuntu::location::Optional<T>::value_type>::encode_argument(writer, *in);
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Optional<T>& in)
    {
        bool has_value{false};
        Codec<bool>::decode_argument(reader, has_value);
        if (has_value)
        {
            typename com::ubuntu::location::Optional<T>::value_type value;
            Codec<typename com::ubuntu::location::Optional<T>::value_type>::decode_argument(reader, value);
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
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::units::Quantity<T>& in)
    {
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::encode_argument(writer, in.value());
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::units::Quantity<T>& in)
    {
        typename com::ubuntu::location::units::Quantity<T>::value_type value;
        Codec<typename com::ubuntu::location::units::Quantity<T>::value_type>::decode_argument(reader, value);
        in = com::ubuntu::location::units::Quantity<T>::from_value(value);
    }
};

template<typename T, typename U>
struct Codec<com::ubuntu::location::wgs84::Coordinate<T,U>>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::encode_argument(writer, in.value);
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::wgs84::Coordinate<T, U>& in)
    {
        Codec<com::ubuntu::location::units::Quantity<U>>::decode_argument(reader, in.value);
    }
};

template<>
struct Codec<com::ubuntu::location::Position>
{
    typedef com::ubuntu::location::Position::Accuracy::Horizontal HorizontalAccuracy;
    typedef com::ubuntu::location::Position::Accuracy::Vertical VerticalAccuracy;

    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Position& in)
    {
        Codec<com::ubuntu::location::wgs84::Latitude>::encode_argument(writer, in.latitude);
        Codec<com::ubuntu::location::wgs84::Longitude>::encode_argument(writer, in.longitude);
        Codec<com::ubuntu::location::Optional<com::ubuntu::location::wgs84::Altitude>>::encode_argument(writer, in.altitude);

        Codec<com::ubuntu::location::Optional<HorizontalAccuracy>>::encode_argument(writer, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::encode_argument(writer, in.accuracy.vertical);
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Position& in)
    {
        Codec<com::ubuntu::location::wgs84::Latitude>::decode_argument(reader, in.latitude);
        Codec<com::ubuntu::location::wgs84::Longitude>::decode_argument(reader, in.longitude);
        Codec<com::ubuntu::location::Optional<com::ubuntu::location::wgs84::Altitude>>::decode_argument(reader, in.altitude);

        Codec<com::ubuntu::location::Optional<HorizontalAccuracy>>::decode_argument(reader, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::decode_argument(reader, in.accuracy.vertical);
    }
};


namespace helper
{
template<>
struct TypeMapper<com::ubuntu::location::SpaceVehicle::Key>
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
                helper::TypeMapper<std::uint32_t>::signature() +
                helper::TypeMapper<std::uint32_t>::signature();
        return s;
    }
};
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

    inline static std::string signature()
    {
        std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                helper::TypeMapper<com::ubuntu::location::SpaceVehicle::Key>::signature() +
                helper::TypeMapper<float>::signature() +
                helper::TypeMapper<bool>::signature() +
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
struct Codec<com::ubuntu::location::SpaceVehicle::Key>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::SpaceVehicle::Key& in)
    {
        writer.push_uint32(static_cast<std::uint32_t>(in.type));
        writer.push_uint32(in.id);
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::SpaceVehicle::Key& in)
    {
        in.type = static_cast<com::ubuntu::location::SpaceVehicle::Type>(reader.pop_uint32());
        in.id = reader.pop_uint32();
    }
};

template<>
struct Codec<com::ubuntu::location::SpaceVehicle>
{
    inline static void encode_argument(Message::Writer& writer, const com::ubuntu::location::SpaceVehicle& in)
    {
        auto sub = writer.open_structure();

        Codec<com::ubuntu::location::SpaceVehicle::Key>::encode_argument(sub, in.key);
        sub.push_floating_point(in.snr);
        sub.push_boolean(in.has_almanac_data);
        sub.push_boolean(in.has_ephimeris_data);
        sub.push_boolean(in.used_in_fix);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::encode_argument(sub, in.azimuth);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::encode_argument(sub, in.elevation);

        writer.close_structure(std::move(sub));
    }

    inline static void decode_argument(Message::Reader& reader, com::ubuntu::location::SpaceVehicle& in)
    {
        auto sub = reader.pop_structure();

        Codec<com::ubuntu::location::SpaceVehicle::Key>::decode_argument(sub, in.key);
        in.snr = sub.pop_floating_point();
        in.has_almanac_data = sub.pop_boolean();
        in.has_ephimeris_data = sub.pop_boolean();
        in.used_in_fix = sub.pop_boolean();
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::decode_argument(sub, in.azimuth);
        Codec<com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle>>::decode_argument(sub, in.elevation);
    }
};

namespace helper
{
template<>
struct TypeMapper<std::map<com::ubuntu::location::SpaceVehicle::Key, com::ubuntu::location::SpaceVehicle>>
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
        static const std::string s = DBUS_TYPE_ARRAY_AS_STRING + TypeMapper<com::ubuntu::location::SpaceVehicle>::signature();
        return s;
    }
};
}
template<>
struct Codec<std::map<com::ubuntu::location::SpaceVehicle::Key, com::ubuntu::location::SpaceVehicle>>
{
    inline static void encode_argument(Message::Writer& writer, const std::map<com::ubuntu::location::SpaceVehicle::Key, com::ubuntu::location::SpaceVehicle>& arg)
    {
        types::Signature signature(helper::TypeMapper<com::ubuntu::location::SpaceVehicle>::signature());
        auto sub = writer.open_array(signature);

        for(const auto& element : arg)
        {
            Codec<com::ubuntu::location::SpaceVehicle>::encode_argument(sub, element.second);
        }

        writer.close_array(std::move(sub));
    }

    inline static void decode_argument(Message::Reader& reader, std::map<com::ubuntu::location::SpaceVehicle::Key, com::ubuntu::location::SpaceVehicle>& out)
    {
        auto sub = reader.pop_array();
        while (sub.type() != ArgumentType::invalid)
        {
            com::ubuntu::location::SpaceVehicle sv;
            Codec<com::ubuntu::location::SpaceVehicle>::decode_argument(sub, sv);
            out.insert(std::make_pair(sv.key, sv));
        }
    }
};

template<>
struct Codec<com::ubuntu::location::Criteria>
{
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Length> HorizontalAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Length> VerticalAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::Velocity> VelocityAccuracy;
    typedef com::ubuntu::location::units::Quantity<com::ubuntu::location::units::PlaneAngle> HeadingAccuracy;

    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Criteria& in)
    {
        Codec<bool>::encode_argument(writer, in.requires.position);
        Codec<bool>::encode_argument(writer, in.requires.altitude);
        Codec<bool>::encode_argument(writer, in.requires.heading);
        Codec<bool>::encode_argument(writer, in.requires.velocity);

        Codec<HorizontalAccuracy>::encode_argument(writer, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::encode_argument(writer, in.accuracy.vertical);
        Codec<com::ubuntu::location::Optional<VelocityAccuracy>>::encode_argument(writer, in.accuracy.velocity);
        Codec<com::ubuntu::location::Optional<HeadingAccuracy>>::encode_argument(writer, in.accuracy.heading);
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Criteria& in)
    {
        Codec<bool>::decode_argument(reader, in.requires.position);
        Codec<bool>::decode_argument(reader, in.requires.altitude);
        Codec<bool>::decode_argument(reader, in.requires.heading);
        Codec<bool>::decode_argument(reader, in.requires.velocity);

        Codec<HorizontalAccuracy>::decode_argument(reader, in.accuracy.horizontal);
        Codec<com::ubuntu::location::Optional<VerticalAccuracy>>::decode_argument(reader, in.accuracy.vertical);
        Codec<com::ubuntu::location::Optional<VelocityAccuracy>>::decode_argument(reader, in.accuracy.velocity);
        Codec<com::ubuntu::location::Optional<HeadingAccuracy>>::decode_argument(reader, in.accuracy.heading);
    }
};

template<>
struct Codec<com::ubuntu::location::Provider::Features>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Provider::Features& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Provider::Features& in)
    {
        in = static_cast<com::ubuntu::location::Provider::Features>(reader.pop_int32());
    }
};

template<>
struct Codec<com::ubuntu::location::Provider::Requirements>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Provider::Requirements& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Provider::Requirements& in)
    {
        in = static_cast<com::ubuntu::location::Provider::Requirements>(reader.pop_int32());
    }
};

template<>
struct Codec<com::ubuntu::location::WifiAndCellIdReportingState>
{
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::WifiAndCellIdReportingState& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::WifiAndCellIdReportingState& in)
    {
        in = static_cast<com::ubuntu::location::WifiAndCellIdReportingState>(reader.pop_int32());
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
    static void encode_argument(Message::Writer& writer, const com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::encode_argument(writer, in.value);
        Codec<int64_t>::encode_argument(writer, in.when.time_since_epoch().count());
    }

    static void decode_argument(Message::Reader& reader, com::ubuntu::location::Update<T>& in)
    {
        Codec<T>::decode_argument(reader, in.value);
        in.when = com::ubuntu::location::Clock::Timestamp(com::ubuntu::location::Clock::Duration(reader.pop_int64()));
    }    
};
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CODEC_H_
