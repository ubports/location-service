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
#ifndef LOCATION_CODEC_H_
#define LOCATION_CODEC_H_

#include <location/criteria.h>
#include <location/heading.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/space_vehicle.h>
#include <location/update.h>
#include <location/velocity.h>
#include <location/service.h>
#include <location/service/state.h>
#include <location/units/units.h>
#include <location/wgs84/altitude.h>
#include <location/wgs84/latitude.h>
#include <location/wgs84/longitude.h>

#include <core/dbus/codec.h>

#include <sstream>

namespace core
{
namespace dbus
{
namespace helper
{
template<>
struct TypeMapper<location::Service::State>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::string;
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
        static const std::string s = TypeMapper<std::string>::signature();
        return s;
    }
};
}

template<>
struct Codec<location::Service::State>
{
    static void encode_argument(Message::Writer& writer, const location::Service::State& in)
    {
        std::stringstream ss; ss << in; auto s = ss.str();
        writer.push_stringn(s.c_str(), s.size());
    }

    static void decode_argument(Message::Reader& reader, location::Service::State& in)
    {
        auto s = reader.pop_string();
        std::stringstream ss{s}; ss >> in;
    }
};

namespace helper
{
template<>
struct TypeMapper<location::service::State>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::string;
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
        static const std::string s = TypeMapper<std::string>::signature();
        return s;
    }
};
}

template<>
struct Codec<location::service::State>
{
    static void encode_argument(Message::Writer& writer, const location::service::State& in)
    {
        std::stringstream ss; ss << in; auto s = ss.str();
        writer.push_stringn(s.c_str(), s.size());
    }

    static void decode_argument(Message::Reader& reader, location::service::State& in)
    {
        auto s = reader.pop_string();
        std::stringstream ss{s}; ss >> in;
    }
};

namespace helper
{
template<typename T>
struct TypeMapper<location::units::Quantity<T>>
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
struct Codec<location::Optional<T>>
{
    static void encode_argument(Message::Writer& writer, const location::Optional<T>& in)
    {
        bool has_value{in};
        Codec<bool>::encode_argument(writer, has_value);
        if (has_value)
            Codec<typename location::Optional<T>::value_type>::encode_argument(writer, *in);
    }

    static void decode_argument(Message::Reader& reader, location::Optional<T>& in)
    {
        bool has_value{false};
        Codec<bool>::decode_argument(reader, has_value);
        if (has_value)
        {
            typename location::Optional<T>::value_type value;
            Codec<typename location::Optional<T>::value_type>::decode_argument(reader, value);
            in = value;
        } else
        {
            in.reset();
        }
    }
};

template<typename T>
struct Codec<location::units::Quantity<T>>
{
    static void encode_argument(Message::Writer& writer, const location::units::Quantity<T>& in)
    {
        Codec<typename location::units::Quantity<T>::value_type>::encode_argument(writer, in.value());
    }

    static void decode_argument(Message::Reader& reader, location::units::Quantity<T>& in)
    {
        typename location::units::Quantity<T>::value_type value;
        Codec<typename location::units::Quantity<T>::value_type>::decode_argument(reader, value);
        in = location::units::Quantity<T>::from_value(value);
    }
};

template<typename T, typename U>
struct Codec<location::wgs84::Coordinate<T,U>>
{
    static void encode_argument(Message::Writer& writer, const location::wgs84::Coordinate<T, U>& in)
    {
        Codec<location::units::Quantity<U>>::encode_argument(writer, in.value);
    }

    static void decode_argument(Message::Reader& reader, location::wgs84::Coordinate<T, U>& in)
    {
        Codec<location::units::Quantity<U>>::decode_argument(reader, in.value);
    }
};

template<>
struct Codec<location::Position>
{
    typedef location::Position::Accuracy::Horizontal HorizontalAccuracy;
    typedef location::Position::Accuracy::Vertical VerticalAccuracy;

    static void encode_argument(Message::Writer& writer, const location::Position& in)
    {
        Codec<location::wgs84::Latitude>::encode_argument(writer, in.latitude);
        Codec<location::wgs84::Longitude>::encode_argument(writer, in.longitude);
        Codec<location::Optional<location::wgs84::Altitude>>::encode_argument(writer, in.altitude);

        Codec<location::Optional<HorizontalAccuracy>>::encode_argument(writer, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::encode_argument(writer, in.accuracy.vertical);
    }

    static void decode_argument(Message::Reader& reader, location::Position& in)
    {
        Codec<location::wgs84::Latitude>::decode_argument(reader, in.latitude);
        Codec<location::wgs84::Longitude>::decode_argument(reader, in.longitude);
        Codec<location::Optional<location::wgs84::Altitude>>::decode_argument(reader, in.altitude);

        Codec<location::Optional<HorizontalAccuracy>>::decode_argument(reader, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::decode_argument(reader, in.accuracy.vertical);
    }
};


namespace helper
{
template<>
struct TypeMapper<location::SpaceVehicle::Key>
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
struct TypeMapper<location::SpaceVehicle>
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
                helper::TypeMapper<location::SpaceVehicle::Key>::signature() +
                helper::TypeMapper<float>::signature() +
                helper::TypeMapper<bool>::signature() +
                helper::TypeMapper<bool>::signature() +
                helper::TypeMapper<bool>::signature() +
                helper::TypeMapper<location::units::Quantity<location::units::PlaneAngle>>::signature() +
                helper::TypeMapper<location::units::Quantity<location::units::PlaneAngle>>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<location::SpaceVehicle::Key>
{
    static void encode_argument(Message::Writer& writer, const location::SpaceVehicle::Key& in)
    {
        writer.push_uint32(static_cast<std::uint32_t>(in.type));
        writer.push_uint32(in.id);
    }

    static void decode_argument(Message::Reader& reader, location::SpaceVehicle::Key& in)
    {
        in.type = static_cast<location::SpaceVehicle::Type>(reader.pop_uint32());
        in.id = reader.pop_uint32();
    }
};

template<>
struct Codec<location::SpaceVehicle>
{
    inline static void encode_argument(Message::Writer& writer, const location::SpaceVehicle& in)
    {
        auto sub = writer.open_structure();

        Codec<location::SpaceVehicle::Key>::encode_argument(sub, in.key);
        sub.push_floating_point(in.snr);
        sub.push_boolean(in.has_almanac_data);
        sub.push_boolean(in.has_ephimeris_data);
        sub.push_boolean(in.used_in_fix);
        Codec<location::units::Quantity<location::units::PlaneAngle>>::encode_argument(sub, in.azimuth);
        Codec<location::units::Quantity<location::units::PlaneAngle>>::encode_argument(sub, in.elevation);

        writer.close_structure(std::move(sub));
    }

    inline static void decode_argument(Message::Reader& reader, location::SpaceVehicle& in)
    {
        auto sub = reader.pop_structure();

        Codec<location::SpaceVehicle::Key>::decode_argument(sub, in.key);
        in.snr = sub.pop_floating_point();
        in.has_almanac_data = sub.pop_boolean();
        in.has_ephimeris_data = sub.pop_boolean();
        in.used_in_fix = sub.pop_boolean();
        Codec<location::units::Quantity<location::units::PlaneAngle>>::decode_argument(sub, in.azimuth);
        Codec<location::units::Quantity<location::units::PlaneAngle>>::decode_argument(sub, in.elevation);
    }
};

namespace helper
{
template<>
struct TypeMapper<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>
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
        static const std::string s = DBUS_TYPE_ARRAY_AS_STRING + TypeMapper<location::SpaceVehicle>::signature();
        return s;
    }
};
}
template<>
struct Codec<std::map<location::SpaceVehicle::Key, location::SpaceVehicle>>
{
    inline static void encode_argument(Message::Writer& writer, const std::map<location::SpaceVehicle::Key, location::SpaceVehicle>& arg)
    {
        types::Signature signature(helper::TypeMapper<location::SpaceVehicle>::signature());
        auto sub = writer.open_array(signature);

        for(const auto& element : arg)
        {
            Codec<location::SpaceVehicle>::encode_argument(sub, element.second);
        }

        writer.close_array(std::move(sub));
    }

    inline static void decode_argument(Message::Reader& reader, std::map<location::SpaceVehicle::Key, location::SpaceVehicle>& out)
    {
        auto sub = reader.pop_array();
        while (sub.type() != ArgumentType::invalid)
        {
            location::SpaceVehicle sv;
            Codec<location::SpaceVehicle>::decode_argument(sub, sv);
            out.insert(std::make_pair(sv.key, sv));
        }
    }
};

template<>
struct Codec<location::Criteria>
{
    typedef location::units::Quantity<location::units::Length> HorizontalAccuracy;
    typedef location::units::Quantity<location::units::Length> VerticalAccuracy;
    typedef location::units::Quantity<location::units::Velocity> VelocityAccuracy;
    typedef location::units::Quantity<location::units::PlaneAngle> HeadingAccuracy;

    static void encode_argument(Message::Writer& writer, const location::Criteria& in)
    {
        Codec<bool>::encode_argument(writer, in.requires.position);
        Codec<bool>::encode_argument(writer, in.requires.altitude);
        Codec<bool>::encode_argument(writer, in.requires.heading);
        Codec<bool>::encode_argument(writer, in.requires.velocity);

        Codec<HorizontalAccuracy>::encode_argument(writer, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::encode_argument(writer, in.accuracy.vertical);
        Codec<location::Optional<VelocityAccuracy>>::encode_argument(writer, in.accuracy.velocity);
        Codec<location::Optional<HeadingAccuracy>>::encode_argument(writer, in.accuracy.heading);
    }

    static void decode_argument(Message::Reader& reader, location::Criteria& in)
    {
        Codec<bool>::decode_argument(reader, in.requires.position);
        Codec<bool>::decode_argument(reader, in.requires.altitude);
        Codec<bool>::decode_argument(reader, in.requires.heading);
        Codec<bool>::decode_argument(reader, in.requires.velocity);

        Codec<HorizontalAccuracy>::decode_argument(reader, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::decode_argument(reader, in.accuracy.vertical);
        Codec<location::Optional<VelocityAccuracy>>::decode_argument(reader, in.accuracy.velocity);
        Codec<location::Optional<HeadingAccuracy>>::decode_argument(reader, in.accuracy.heading);
    }
};

template<>
struct Codec<location::Provider::Features>
{
    static void encode_argument(Message::Writer& writer, const location::Provider::Features& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, location::Provider::Features& in)
    {
        in = static_cast<location::Provider::Features>(reader.pop_int32());
    }
};

template<>
struct Codec<location::Provider::Requirements>
{
    static void encode_argument(Message::Writer& writer, const location::Provider::Requirements& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, location::Provider::Requirements& in)
    {
        in = static_cast<location::Provider::Requirements>(reader.pop_int32());
    }
};

template<>
struct Codec<location::WifiAndCellIdReportingState>
{
    static void encode_argument(Message::Writer& writer, const location::WifiAndCellIdReportingState& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, location::WifiAndCellIdReportingState& in)
    {
        in = static_cast<location::WifiAndCellIdReportingState>(reader.pop_int32());
    }
};

namespace helper
{
template<typename T>
struct TypeMapper<location::Update<T>>
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
struct Codec<location::Update<T>>
{
    static void encode_argument(Message::Writer& writer, const location::Update<T>& in)
    {
        Codec<T>::encode_argument(writer, in.value);
        Codec<int64_t>::encode_argument(writer, in.when.time_since_epoch().count());
    }

    static void decode_argument(Message::Reader& reader, location::Update<T>& in)
    {
        Codec<T>::decode_argument(reader, in.value);
        in.when = location::Clock::Timestamp(location::Clock::Duration(reader.pop_int64()));
    }    
};
}
}

#endif // LOCATION_CODEC_H_
