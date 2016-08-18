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
#include <location/position.h>

#include <bitset>
#include <ostream>

struct location::Position::Accuracy::Private
{
    location::Optional<location::units::Meters> horizontal_;
    location::Optional<location::units::Meters> vertical_;
};

location::Position::Accuracy::Accuracy() : d{new Private{}}
{
}

location::Position::Accuracy::~Accuracy() = default;

location::Position::Accuracy::Accuracy(const Accuracy& rhs) : d{new Private{*rhs.d}}
{
}

location::Position::Accuracy::Accuracy(Accuracy&& rhs) : d{std::move(rhs.d)}
{
}

location::Position::Accuracy& location::Position::Accuracy::operator=(const Accuracy& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::Position::Accuracy& location::Position::Accuracy::operator=(Accuracy&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

bool location::Position::Accuracy::operator==(const location::Position::Accuracy& rhs) const
{
    return std::tie(d->horizontal_, d->vertical_) == std::tie(rhs.d->horizontal_, rhs.d->vertical_);
}

const location::Optional<location::units::Meters>& location::Position::Accuracy::horizontal() const
{
    return d->horizontal_;
}

location::Optional<location::units::Meters>& location::Position::Accuracy::horizontal()
{
    return d->horizontal_;
}

location::Position::Accuracy& location::Position::Accuracy::horizontal(const units::Meters& value)
{
    d->horizontal_ = value;
    return *this;
}

location::Position::Accuracy location::Position::Accuracy::horizontal(const units::Meters& value) const
{
    auto copy = *this;
    copy.d->horizontal_ = value;
    return copy;
}

const location::Optional<location::units::Meters>& location::Position::Accuracy::vertical() const
{
    return d->vertical_;
}

location::Optional<location::units::Meters>& location::Position::Accuracy::vertical()
{
    return d->vertical_;
}

location::Position::Accuracy& location::Position::Accuracy::vertical(const units::Meters& value)
{
    d->vertical_ = value;
    return *this;
}

location::Position::Accuracy location::Position::Accuracy::vertical(const units::Meters& value) const
{
    auto copy = *this;
    copy.d->vertical_ = value;
    return copy;
}

struct location::Position::Private
{
    Private(const units::Degrees& latitude, const units::Degrees& longitude)
        : latitude_{latitude}, longitude_{longitude}
    {
    }

    units::Degrees latitude_;
    units::Degrees longitude_;
    location::Optional<units::Meters> altitude_;
    location::Position::Accuracy accuracy_;
};

location::Position::Position(const units::Degrees& latitude, const units::Degrees& longitude)
    : d{new Private{latitude, longitude}}
{
}

location::Position::Position::~Position() = default;

location::Position::Position(const location::Position& rhs) : d{new Private{*rhs.d}}
{
}

location::Position::Position(location::Position&& rhs) : d{std::move(rhs.d)}
{
}

location::Position& location::Position::Position::operator=(const location::Position& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::Position& location::Position::Position::operator=(location::Position&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

bool location::Position::operator==(const location::Position& rhs) const
{
    return std::tie(d->latitude_, d->longitude_, d->altitude_, d->accuracy_) ==
           std::tie(rhs.d->latitude_, rhs.d->longitude_, rhs.d->altitude_, rhs.d->accuracy_);
}

bool location::Position::operator!=(const location::Position& rhs) const
{
    return std::tie(d->latitude_, d->longitude_, d->altitude_, d->accuracy_) !=
            std::tie(rhs.d->latitude_, rhs.d->longitude_, rhs.d->altitude_, rhs.d->accuracy_);
}

const location::units::Degrees& location::Position::latitude() const
{
    return d->latitude_;
}

location::units::Degrees& location::Position::latitude()
{
    return d->latitude_;
}

location::Position& location::Position::latitude(const units::Degrees& value)
{
    d->latitude_ = value;
    return *this;
}

location::Position location::Position::latitude(const units::Degrees& value) const
{
    auto copy = *this;
    copy.d->latitude_ = value;
    return copy;
}

const location::units::Degrees& location::Position::longitude() const
{
    return d->longitude_;
}

location::units::Degrees& location::Position::longitude()
{
    return d->longitude_;
}

location::Position& location::Position::longitude(const units::Degrees& value)
{
    d->longitude_ = value;
    return *this;
}

location::Position location::Position::longitude(const units::Degrees& value) const
{
    auto copy = *this;
    copy.d->longitude_ = value;
    return copy;
}

const location::Optional<location::units::Meters>& location::Position::altitude() const
{
    return d->altitude_;
}

location::Optional<location::units::Meters>& location::Position::altitude()
{
    return d->altitude_;
}

location::Position& location::Position::altitude(const units::Meters& value)
{
    d->altitude_ = value;
    return *this;
}

location::Position location::Position::altitude(const units::Meters& value) const
{
    auto copy = *this;
    copy.d->altitude_ = value;
    return copy;
}

const location::Position::Accuracy& location::Position::accuracy() const
{
    return d->accuracy_;
}

location::Position::Accuracy& location::Position::accuracy()
{
    return d->accuracy_;
}

std::ostream& location::operator<<(std::ostream& out, const location::Position& position)
{
    out << "Position("
        << "lat: " << position.latitude() << ", "
        << "lon: " << position.longitude() << ", ";
    out << "alt: ";
    if (position.altitude())
        out << *position.altitude();
    else
        out << "n/a";
    out << ", ";
    out << "hor.acc.: ";
    if (position.accuracy().horizontal())
        out << *position.accuracy().horizontal();
    else
        out << "n/a";
    out << ", ";
    out << "ver.acc.: ";
    if (position.accuracy().vertical())
        out << *position.accuracy().vertical();
    else
        out << "n/a";
    out << ")";
    return out;
}

location::units::Meters location::haversine_distance(const location::Position& p1, const location::Position& p2)
{
    static const units::Meters radius_of_earth {6371 * units::kilo * units::meters};
    auto dLat = p2.latitude() - p1.latitude();
    auto dLon = p2.longitude() - p1.longitude();
    auto a =
        std::pow(units::sin(dLat/2.), 2) +
        std::pow(units::sin(dLon/2.), 2) * units::cos(p1.latitude()) * units::cos(p2.latitude());

    auto c = 2. * std::atan2(std::sqrt(a), std::sqrt(1.-a));
    return radius_of_earth * c;
}
