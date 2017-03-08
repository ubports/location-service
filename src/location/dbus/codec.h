/*
 * Copyright © 2017 Canonical Ltd.
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

#ifndef LOCATION_DBUS_CODEC_H_
#define LOCATION_DBUS_CODEC_H_

#include <location/criteria.h>
#include <location/optional.h>
#include <location/permission_manager.h>
#include <location/position.h>
#include <location/provider.h>
#include <location/update.h>
#include <location/units/units.h>

#include <glib.h>

namespace location
{
namespace dbus
{

template<typename T>
struct Codec
{
    static GVariant* encode(const T& value);
    static Optional<T> decode(GVariant* variant);
};

template<>
struct Codec<Credentials>
{
    static GVariant* encode(const Credentials& value);
    static Optional<Credentials> decode(GVariant* variant);
};

template<>
struct Codec<Criteria>
{
    static GVariant* encode(const Criteria& value);
    static Optional<Criteria> decode(GVariant* variant);
};

template<>
struct Codec<Features>
{
    static GVariant* encode(const Features& value);
    static Optional<Features> decode(GVariant* variant);
};

template<>
struct Codec<Provider::Requirements>
{
    static GVariant* encode(const Provider::Requirements& value);
    static Optional<Provider::Requirements> decode(GVariant* variant);
};

template<>
struct Codec<location::Update<location::Position>>
{
    static GVariant* encode(const location::Update<location::Position>& value);
    static Optional<location::Update<location::Position>> decode(GVariant* variant);
};

template<>
struct Codec<location::Update<units::Degrees>>
{
    static GVariant* encode(const location::Update<units::Degrees>& value);
    static Optional<location::Update<units::Degrees>> decode(GVariant* variant);
};

template<>
struct Codec<location::Update<units::Meters>>
{
    static GVariant* encode(const location::Update<units::Meters>& value);
    static Optional<location::Update<units::Meters>> decode(GVariant* variant);
};

template<>
struct Codec<location::Update<units::MetersPerSecond>>
{
    static GVariant* encode(const location::Update<units::MetersPerSecond>& value);
    static Optional<location::Update<units::MetersPerSecond>> decode(GVariant* variant);
};

template<>
struct Codec<location::Event>
{
    static GVariant* encode(const location::Event& value);
};

template<>
struct Codec<location::Event::Ptr>
{
    static GVariant* encode(const location::Event::Ptr& value);
    static Optional<location::Event::Ptr> decode(GVariant* variant);
};

template<typename T>
GVariant* encode(const T& value)
{
    return Codec<T>::encode(value);
}

template<typename T>
Optional<T> decode(GVariant* variant)
{
    return Codec<T>::decode(variant);
}

}  // namespace dbus
}  // namespace location

#endif  // LOCATION_DBUS_CODEC_H_
