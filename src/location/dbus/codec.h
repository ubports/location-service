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
#include <location/features.h>
#include <location/heading.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/space_vehicle.h>
#include <location/update.h>
#include <location/velocity.h>
#include <location/service.h>
#include <location/events/all.h>
#include <location/events/registry.h>
#include <location/events/reference_position_updated.h>
#include <location/events/wifi_and_cell_id_reporting_state_changed.h>
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

template<typename T>
struct TypeMapper<location::Optional<T>>
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
        static const std::string s = DBUS_STRUCT_BEGIN_CHAR_AS_STRING + TypeMapper<bool>::signature() + DBUS_TYPE_VARIANT_AS_STRING + DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<typename T>
struct Codec<location::Optional<T>>
{
    static void encode_argument(Message::Writer& writer, const location::Optional<T>& in)
    {
        auto sw = writer.open_structure();
        {
            bool has_value{in};
            Codec<bool>::encode_argument(sw, has_value);

            if (has_value)
            {
                auto vw = sw.open_variant(types::Signature{helper::TypeMapper<T>::signature()});
                Codec<T>::encode_argument(vw, *in);
                sw.close_variant(std::move(vw));
            }
            else
            {
                auto vw = sw.open_variant(types::Signature{helper::TypeMapper<bool>::signature()});
                Codec<bool>::encode_argument(vw, false);
                sw.close_variant(std::move(vw));
            }
        }
        writer.close_structure(std::move(sw));
    }

    static void decode_argument(Message::Reader& reader, location::Optional<T>& in)
    {
        auto sr = reader.pop_structure();
        {
            bool has_value{false};
            Codec<bool>::decode_argument(sr, has_value);
            auto vr = sr.pop_variant();
            if (has_value)
            {
                T value;
                Codec<T>::decode_argument(vr, value);
                in = value;
            } else
            {
                in.reset();
            }
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

namespace helper
{
template<typename T, typename U>
struct TypeMapper<location::wgs84::Coordinate<T, U>>
{
    constexpr static ArgumentType type_value()
    {
        return TypeMapper<location::units::Quantity<U>>::type_value();
    }

    constexpr static bool is_basic_type()
    {
        return true;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s = TypeMapper<location::units::Quantity<U>>::signature();
        return s;
    }
};
}

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

namespace helper
{
template<>
struct TypeMapper<location::Position>
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
                TypeMapper<location::wgs84::Latitude>::signature() +
                TypeMapper<location::wgs84::Longitude>::signature() +
                TypeMapper<location::Optional<location::wgs84::Altitude>>::signature() +
                TypeMapper<location::Optional<location::Position::Accuracy::Horizontal>>::signature() +
                TypeMapper<location::Optional<location::Position::Accuracy::Vertical>>::signature() +
                DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<location::Position>
{
    typedef location::Position::Accuracy::Horizontal HorizontalAccuracy;
    typedef location::Position::Accuracy::Vertical VerticalAccuracy;

    static void encode_argument(Message::Writer& writer, const location::Position& in)
    {
        auto vw = writer.open_structure();
        {
            Codec<location::wgs84::Latitude>::encode_argument(vw, in.latitude);
            Codec<location::wgs84::Longitude>::encode_argument(vw, in.longitude);
            Codec<location::Optional<location::wgs84::Altitude>>::encode_argument(vw, in.altitude);

            Codec<location::Optional<HorizontalAccuracy>>::encode_argument(vw, in.accuracy.horizontal);
            Codec<location::Optional<VerticalAccuracy>>::encode_argument(vw, in.accuracy.vertical);
        }
        writer.close_structure(std::move(vw));
    }

    static void decode_argument(Message::Reader& reader, location::Position& in)
    {
        auto vr = reader.pop_structure();
        {
            Codec<location::wgs84::Latitude>::decode_argument(vr, in.latitude);
            Codec<location::wgs84::Longitude>::decode_argument(vr, in.longitude);
            Codec<location::Optional<location::wgs84::Altitude>>::decode_argument(vr, in.altitude);

            Codec<location::Optional<HorizontalAccuracy>>::decode_argument(vr, in.accuracy.horizontal);
            Codec<location::Optional<VerticalAccuracy>>::decode_argument(vr, in.accuracy.vertical);
        }
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
struct Codec<location::Features>
{
    static void encode_argument(Message::Writer& writer, const location::Features& in)
    {
        writer.push_int32(static_cast<std::int32_t>(in));
    }

    static void decode_argument(Message::Reader& reader, location::Features& in)
    {
        in = static_cast<location::Features>(reader.pop_int32());
    }
};

namespace helper
{
template<>
struct TypeMapper<location::Criteria>
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
                TypeMapper<std::int32_t>::signature() +
                TypeMapper<location::Optional<location::Position::Accuracy::Horizontal>>::signature() +
                TypeMapper<location::Optional<location::Position::Accuracy::Vertical>>::signature() +
                TypeMapper<location::Optional<location::units::Quantity<location::units::Velocity>>>::signature() +
                TypeMapper<location::Optional<location::units::Quantity<location::units::PlaneAngle>>>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<>
struct Codec<location::Criteria>
{
    typedef location::units::Quantity<location::units::Length> HorizontalAccuracy;
    typedef location::units::Quantity<location::units::Length> VerticalAccuracy;
    typedef location::units::Quantity<location::units::Velocity> VelocityAccuracy;
    typedef location::units::Quantity<location::units::PlaneAngle> HeadingAccuracy;

    static void encode_argument(Message::Writer& writer, const location::Criteria& in)
    {
        Codec<location::Features>::encode_argument(writer, in.requirements);

        Codec<location::Optional<HorizontalAccuracy>>::encode_argument(writer, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::encode_argument(writer, in.accuracy.vertical);
        Codec<location::Optional<VelocityAccuracy>>::encode_argument(writer, in.accuracy.velocity);
        Codec<location::Optional<HeadingAccuracy>>::encode_argument(writer, in.accuracy.heading);
    }

    static void decode_argument(Message::Reader& reader, location::Criteria& in)
    {
        Codec<location::Features>::decode_argument(reader, in.requirements);

        Codec<location::Optional<HorizontalAccuracy>>::decode_argument(reader, in.accuracy.horizontal);
        Codec<location::Optional<VerticalAccuracy>>::decode_argument(reader, in.accuracy.vertical);
        Codec<location::Optional<VelocityAccuracy>>::decode_argument(reader, in.accuracy.velocity);
        Codec<location::Optional<HeadingAccuracy>>::decode_argument(reader, in.accuracy.heading);
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

namespace helper
{
template<>
struct TypeMapper<location::WifiAndCellIdReportingState>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::int32;
    }
    constexpr static bool is_basic_type()
    {
        return true;
    }

    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        static const std::string s = helper::TypeMapper<std::int32_t>::signature();
        return s;
    }
};
}

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
                DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
                helper::TypeMapper<T>::signature() +
                helper::TypeMapper<int64_t>::signature() +
                DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<typename T>
struct Codec<location::Update<T>>
{
    static void encode_argument(Message::Writer& writer, const location::Update<T>& in)
    {
        auto sw = writer.open_structure();
        {
            Codec<T>::encode_argument(sw, in.value);
            Codec<int64_t>::encode_argument(sw, in.when.time_since_epoch().count());
        }
        writer.close_structure(std::move(sw));
    }

    static void decode_argument(Message::Reader& reader, location::Update<T>& in)
    {
        auto sr = reader.pop_structure();
        Codec<T>::decode_argument(sr, in.value);
        in.when = location::Clock::Timestamp(location::Clock::Duration(sr.pop_int64()));
    }    
};

template<>
struct Codec<location::events::ReferencePositionUpdated>
{
    static void encode_argument(Message::Writer& out, const location::events::ReferencePositionUpdated& event)
    {
        Codec<location::Update<location::Position>>::encode_argument(out, event.update());
    }

    static void decode_argument(Message::Reader& in, location::events::ReferencePositionUpdated& event)
    {
        location::Update<location::Position> update;
        Codec<location::Update<location::Position>>::decode_argument(in, update);
        event = location::events::ReferencePositionUpdated{update};
    }
};

template<>
struct Codec<location::events::WifiAndCellIdReportingStateChanged>
{
    static void encode_argument(Message::Writer& out, const location::events::WifiAndCellIdReportingStateChanged& event)
    {
        Codec<location::WifiAndCellIdReportingState>::encode_argument(out, event.new_state());
    }

    static void decode_argument(Message::Reader& in, location::events::WifiAndCellIdReportingStateChanged& event)
    {
        location::WifiAndCellIdReportingState new_state;
        Codec<location::WifiAndCellIdReportingState>::decode_argument(in, new_state);
        event = location::events::WifiAndCellIdReportingStateChanged{new_state};
    }
};

template<>
struct Codec<location::events::All>
{
    static void encode_argument(Message::Writer& out, const location::events::All& event)
    {
        switch (event.which())
        {
        case 1:
        {
            const auto& ev = boost::get<location::events::ReferencePositionUpdated>(event);
            auto name = location::events::Registry::instance().find(ev.type());
            out.push_stringn(name.c_str(), name.size());
            auto vw = out.open_variant(types::Signature{helper::TypeMapper<location::Update<location::Position>>::signature()});
            {
                Codec<location::events::ReferencePositionUpdated>::encode_argument(vw, ev);
            }
            out.close_variant(std::move(vw));
            break;
        }
        case 2:
        {
            const auto& ev = boost::get<location::events::WifiAndCellIdReportingStateChanged>(event);
            auto name = location::events::Registry::instance().find(ev.type());
            out.push_stringn(name.c_str(), name.size());
            auto vw = out.open_variant(types::Signature{helper::TypeMapper<location::WifiAndCellIdReportingState>::signature()});
            {
                Codec<location::events::WifiAndCellIdReportingStateChanged>::encode_argument(vw, ev);
            }
            out.close_variant(std::move(vw));
            break;
        }
        }
    }

    static void decode_argument(Message::Reader& in, location::events::All& event)
    {
        std::string name{in.pop_string()};

        auto type = location::events::Registry::instance().find(name);

        if (type == location::TypeOf<location::events::ReferencePositionUpdated>::query())
        {
            location::events::ReferencePositionUpdated ev{location::Update<location::Position>{}};
            auto vr = in.pop_variant();
            Codec<location::events::ReferencePositionUpdated>::decode_argument(vr, ev);
            event = ev;
        }
        else if (type == location::TypeOf<location::events::WifiAndCellIdReportingStateChanged>::query())
        {
            location::events::WifiAndCellIdReportingStateChanged ev{location::WifiAndCellIdReportingState::off};
            auto vr = in.pop_variant();
            Codec<location::events::WifiAndCellIdReportingStateChanged>::decode_argument(vr, ev);
            event = ev;
        }
    }
};
}
}

#endif // LOCATION_CODEC_H_
