/*
 * Copyright © 2016 Canonical Ltd.
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

#include <location/space_vehicle.h>

struct location::SpaceVehicle::Key::Private
{
    location::SpaceVehicle::Type type;
    location::SpaceVehicle::Id id;
};

location::SpaceVehicle::Key::Key(Type type, Id id) : d{new Private{type, id}}
{
}

location::SpaceVehicle::Key::Key(const Key& other) : d{new Private{*other.d}}
{
}

location::SpaceVehicle::Key::Key(Key&& other) : d{std::move(other.d)}
{
}

location::SpaceVehicle::Key::~Key() = default;

location::SpaceVehicle::Key& location::SpaceVehicle::Key::operator=(const Key& other)
{
    *d = *other.d;
    return *this;
}

location::SpaceVehicle::Key& location::SpaceVehicle::Key::operator=(Key&& other)
{
    d = std::move(other.d);
    return *this;
}

const location::SpaceVehicle::Type& location::SpaceVehicle::Key::type() const
{
    return d->type;
}

const location::SpaceVehicle::Id& location::SpaceVehicle::Key::id() const
{
    return d->id;
}

struct location::SpaceVehicle::Private
{
    explicit Private(const location::SpaceVehicle::Key& key) : key{key}
    {
    }

    location::SpaceVehicle::Key key;
    location::Optional<float> snr;
    location::Optional<bool> has_almanac_data;
    location::Optional<bool> has_ephimeris_data;
    location::Optional<bool> used_in_fix;
    location::Optional<location::units::Degrees> azimuth;
    location::Optional<location::units::Degrees> elevation;
};

location::SpaceVehicle::SpaceVehicle(const Key& key) : d{new Private{key}}
{
}

location::SpaceVehicle::SpaceVehicle(const SpaceVehicle& other) : d{new Private{*other.d}}
{
}

location::SpaceVehicle::SpaceVehicle(SpaceVehicle&& other) : d{std::move(other.d)}
{
}

location::SpaceVehicle::~SpaceVehicle() = default;

location::SpaceVehicle& location::SpaceVehicle::operator=(const SpaceVehicle& other)
{
    *d = *other.d;
    return *this;
}

location::SpaceVehicle& location::SpaceVehicle::operator=(SpaceVehicle&& other)
{
    d = std::move(other.d);
    return *this;
}

const location::SpaceVehicle::Key& location::SpaceVehicle::key() const
{
    return d->key;
}

location::SpaceVehicle::Key& location::SpaceVehicle::key()
{
    return d->key;
}

location::SpaceVehicle& location::SpaceVehicle::key(const Key& value)
{
    d->key = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::key(const Key& value) const
{
    auto copy = *this;
    copy.d->key = value;
    return copy;
}

const location::Optional<float>& location::SpaceVehicle::snr() const
{
    return d->snr;
}

location::Optional<float>& location::SpaceVehicle::snr()
{
    return d->snr;
}

location::SpaceVehicle& location::SpaceVehicle::snr(float value)
{
    d->snr = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::snr(float value) const
{
    auto copy = *this;
    copy.d->snr = value;
    return *this;
}

const location::Optional<bool>& location::SpaceVehicle::has_almanac_data() const
{
    return d->has_almanac_data;
}

location::Optional<bool>& location::SpaceVehicle::has_almanac_data()
{
    return d->has_almanac_data;
}

location::SpaceVehicle& location::SpaceVehicle::has_almanac_data(bool value)
{
    d->has_almanac_data = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::has_almanac_data(bool value) const
{
    auto copy = *this;
    copy.d->has_almanac_data = value;
    return copy;
}

const location::Optional<bool>& location::SpaceVehicle::has_ephimeris_data() const
{
    return d->has_ephimeris_data;
}

location::Optional<bool>& location::SpaceVehicle::has_ephimeris_data()
{
    return d->has_ephimeris_data;
}

location::SpaceVehicle& location::SpaceVehicle::has_ephimeris_data(bool value)
{
    d->has_ephimeris_data = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::has_ephimeris_data(bool value) const
{
    auto copy = *this;
    copy.d->has_ephimeris_data = value;
    return copy;
}

const location::Optional<bool>& location::SpaceVehicle::used_in_fix() const
{
    return d->used_in_fix;
}

location::Optional<bool>& location::SpaceVehicle::used_in_fix()
{
    return d->used_in_fix;
}

location::SpaceVehicle& location::SpaceVehicle::used_in_fix(bool value)
{
    d->used_in_fix = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::used_in_fix(bool value) const
{
    auto copy = *this;
    copy.d->used_in_fix = value;
    return copy;
}

const location::Optional<location::units::Degrees>& location::SpaceVehicle::azimuth() const
{
    return d->azimuth;
}

location::Optional<location::units::Degrees>& location::SpaceVehicle::azimuth()
{
    return d->azimuth;
}

location::SpaceVehicle& location::SpaceVehicle::azimuth(const units::Degrees& value)
{
    d->azimuth = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::azimuth(const units::Degrees& value) const
{
    auto copy = *this;
    copy.d->azimuth = value;
    return copy;
}

const location::Optional<location::units::Degrees>& location::SpaceVehicle::elevation() const
{
    return d->elevation;
}

location::Optional<location::units::Degrees>& location::SpaceVehicle::elevation()
{
    return d->elevation;
}

location::SpaceVehicle& location::SpaceVehicle::elevation(const units::Degrees& value)
{
    d->elevation = value;
    return *this;
}

location::SpaceVehicle location::SpaceVehicle::elevation(const units::Degrees& value) const
{
    auto copy = *this;
    copy.d->elevation = value;
    return copy;
}

bool location::operator==(const SpaceVehicle& lhs, const SpaceVehicle& rhs)
{
    return std::tie(lhs.key(), lhs.snr(), lhs.has_almanac_data(), lhs.has_ephimeris_data(), lhs.used_in_fix(), lhs.azimuth(), lhs.elevation()) ==
           std::tie(rhs.key(), rhs.snr(), rhs.has_almanac_data(), rhs.has_ephimeris_data(), rhs.used_in_fix(), rhs.azimuth(), rhs.elevation());
}

bool location::operator<(const SpaceVehicle& lhs, const SpaceVehicle& rhs)
{
    return std::tie(lhs.key(), lhs.snr(), lhs.has_almanac_data(), lhs.has_ephimeris_data(), lhs.used_in_fix(), lhs.azimuth(), lhs.elevation()) <
           std::tie(rhs.key(), rhs.snr(), rhs.has_almanac_data(), rhs.has_ephimeris_data(), rhs.used_in_fix(), rhs.azimuth(), rhs.elevation());
}

bool location::operator==(const SpaceVehicle::Key& lhs, const SpaceVehicle::Key& rhs)
{
    return std::tie(lhs.type(), lhs.id()) == std::tie(rhs.type(), rhs.id());
}

bool location::operator<(const SpaceVehicle::Key& lhs, const SpaceVehicle::Key& rhs)
{
    return std::tie(lhs.type(), lhs.id()) < std::tie(rhs.type(), rhs.id());
}

std::ostream& location::operator<<(std::ostream& out, const location::SpaceVehicle::Type& type)
{
    switch(type)
    {
    case SpaceVehicle::Type::unknown: return out << "unknown";
    case SpaceVehicle::Type::beidou: return out << "beidou";
    case SpaceVehicle::Type::galileo: return out << "galileo";
    case SpaceVehicle::Type::glonass: return out << "glonass";
    case SpaceVehicle::Type::gps: return out << "gps";
    case SpaceVehicle::Type::compass: return out << "compass";
    case SpaceVehicle::Type::irnss: return out << "irnss";
    case SpaceVehicle::Type::qzss: return out << "qzss";
    }

    return out;
}

std::ostream& location::operator<<(std::ostream& out, const location::SpaceVehicle::Key& key)
{
    return out << "(" << key.type() << ", " << key.id() << ")";
}

std::ostream& location::operator<<(std::ostream& out, const location::SpaceVehicle& sv)
{
    return out << "("
               << "key: " << sv.key() << ", "
               << "snr: " << sv.snr() << ", "
               << "has_almanac_data: " << sv.has_almanac_data() << ", "
               << "has_ephimeris_data: " << sv.has_ephimeris_data() << ", "
               << "used_in_fix: " << sv.used_in_fix() << ", "
               << "azimuth: " << sv.azimuth() << ", "
               << "elevation: " << sv.elevation()
               << ")";
}
