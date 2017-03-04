/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
 *
 */

#include <location/criteria.h>

#include <iostream>

struct location::Criteria::Accuracy::Private
{
    location::Optional<location::units::Meters> horizontal_;
    location::Optional<location::units::Meters> vertical_;
    location::Optional<location::units::MetersPerSecond> velocity_;
    location::Optional<location::units::Degrees> heading_;
};

location::Criteria::Accuracy::Accuracy() : d{new Private{}}
{
}

location::Criteria::Accuracy::~Accuracy() = default;

location::Criteria::Accuracy::Accuracy(const location::Criteria::Accuracy& rhs) : d{new Private{*rhs.d}}
{
}

location::Criteria::Accuracy::Accuracy(location::Criteria::Accuracy&& rhs) : d{std::move(rhs.d)}
{
}

location::Criteria::Accuracy& location::Criteria::Accuracy::operator=(const location::Criteria::Accuracy& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::Criteria::Accuracy& location::Criteria::Accuracy::operator=(location::Criteria::Accuracy&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

const location::Optional<location::units::Meters>& location::Criteria::Accuracy::horizontal() const
{
    return d->horizontal_;
}

location::Optional<location::units::Meters>& location::Criteria::Accuracy::horizontal()
{
    return d->horizontal_;
}

location::Criteria::Accuracy location::Criteria::Accuracy::horizontal(const units::Meters& value) const
{
    auto copy = *this;
    copy.d->horizontal_ = value;
    return copy;
}

location::Criteria::Accuracy& location::Criteria::Accuracy::horizontal(const units::Meters& value)
{
    d->horizontal_ = value;
    return *this;
}

const location::Optional<location::units::Meters>& location::Criteria::Accuracy::vertical() const
{
    return d->vertical_;
}

location::Optional<location::units::Meters>& location::Criteria::Accuracy::vertical()
{
    return d->vertical_;
}

location::Criteria::Accuracy location::Criteria::Accuracy::vertical(const units::Meters& value) const
{
    auto copy = *this;
    copy.d->vertical_ = value;
    return copy;
}

location::Criteria::Accuracy& location::Criteria::Accuracy::vertical(const units::Meters& value)
{
    d->vertical_ = value;
    return *this;
}

const location::Optional<location::units::MetersPerSecond>& location::Criteria::Accuracy::velocity() const
{
    return d->velocity_;
}

location::Optional<location::units::MetersPerSecond>& location::Criteria::Accuracy::velocity()
{
    return d->velocity_;
}

location::Criteria::Accuracy location::Criteria::Accuracy::velocity(const units::MetersPerSecond& value) const
{
    auto copy = *this;
    copy.d->velocity_ = value;
    return copy;
}

location::Criteria::Accuracy& location::Criteria::Accuracy::velocity(const units::MetersPerSecond& value)
{
    d->velocity_ = value;
    return *this;
}

const location::Optional<location::units::Degrees>& location::Criteria::Accuracy::heading() const
{
    return d->heading_;
}

location::Optional<location::units::Degrees>& location::Criteria::Accuracy::heading()
{
    return d->heading_;
}

location::Criteria::Accuracy location::Criteria::Accuracy::heading(const units::Degrees& value) const
{
    auto copy = *this;
    copy.d->heading_ = value;
    return copy;
}

location::Criteria::Accuracy& location::Criteria::Accuracy::heading(const units::Degrees& value)
{
    d->heading_ = value;
    return *this;
}

struct location::Criteria::Private
{
    explicit Private(const location::Features& requirements)
        : requirements_{requirements}
    {
    }

    location::Features requirements_;
    location::Criteria::Accuracy accuracy_;
};

location::Criteria::Criteria(Features requirements)
    : d{new Private{requirements}}
{
}

location::Criteria::~Criteria() = default;

location::Criteria::Criteria(const Criteria& rhs) : d{new Private{*rhs.d}}
{
}

location::Criteria::Criteria(Criteria&& rhs) : d{std::move(rhs.d)}
{
}

location::Criteria& location::Criteria::operator=(const Criteria& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::Criteria& location::Criteria::operator=(Criteria&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

location::Features location::Criteria::requirements() const
{
    return d->requirements_;
}

location::Features& location::Criteria::requirements()
{
    return d->requirements_;
}

location::Criteria location::Criteria::requirements(Features requirements) const
{
    auto copy = *this;
    copy.d->requirements_ = requirements;
    return copy;
}

location::Criteria& location::Criteria::requirements(Features requirements)
{
    d->requirements_ = requirements;
    return *this;
}

const location::Criteria::Accuracy& location::Criteria::accuracy() const
{
    return d->accuracy_;
}

location::Criteria::Accuracy& location::Criteria::accuracy()
{
    return d->accuracy_;
}

