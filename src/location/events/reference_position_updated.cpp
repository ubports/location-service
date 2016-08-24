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

#include <location/events/reference_position_updated.h>

namespace
{
const location::Event::Type id = location::Event::register_type<location::events::ReferencePositionUpdated>("location::events::ReferencePositionUpdated");
}

struct location::events::ReferencePositionUpdated::Private
{
    explicit Private(const location::Update<location::Position>& update) : update{update}
    {
    }

    location::Update<location::Position> update;
};

location::events::ReferencePositionUpdated::ReferencePositionUpdated(const Update<Position>& update)
    : d{new Private{update}}
{
}

location::events::ReferencePositionUpdated::ReferencePositionUpdated(const ReferencePositionUpdated& rhs)
    : d{new Private{*rhs.d}}
{
}

location::events::ReferencePositionUpdated::ReferencePositionUpdated(ReferencePositionUpdated&& rhs)
    : d{std::move(rhs.d)}
{
}

location::events::ReferencePositionUpdated::~ReferencePositionUpdated() = default;

location::events::ReferencePositionUpdated& location::events::ReferencePositionUpdated::operator=(const ReferencePositionUpdated& rhs)
{
    *d = *rhs.d;
    return *this;
}

location::events::ReferencePositionUpdated& location::events::ReferencePositionUpdated::operator=(ReferencePositionUpdated&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

const location::Update<location::Position>& location::events::ReferencePositionUpdated::update() const
{
    return d->update;
}

location::Event::Type location::events::ReferencePositionUpdated::type() const
{
    return id;
}

location::Event::Type location::TypeOf<location::events::ReferencePositionUpdated>::query()
{
    return id;
}
