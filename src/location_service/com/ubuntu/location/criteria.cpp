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

#include <com/ubuntu/location/criteria.h>

bool com::ubuntu::location::Criteria::satisfies(const com::ubuntu::location::Criteria& rhs) const
{
    bool result = false;

    if (rhs.requires.position)
        result &= requires.position;

    if (rhs.requires.altitude)
        result &= requires.altitude;

    if (rhs.requires.heading)
        result &= requires.heading;

    if (rhs.requires.velocity)
        result &= requires.velocity;

    result &= accuracy.horizontal <= rhs.accuracy.horizontal;

    if (rhs.accuracy.vertical)
        result &= accuracy.vertical && accuracy.vertical <= rhs.accuracy.vertical;

    if (rhs.accuracy.heading)
        result &= accuracy.heading && accuracy.heading <= rhs.accuracy.heading;

    if (rhs.accuracy.velocity)
        result &= accuracy.velocity && accuracy.velocity <= rhs.accuracy.velocity;

    return result;
}

com::ubuntu::location::Criteria com::ubuntu::location::operator+(
        const com::ubuntu::location::Criteria& lhs,
        const com::ubuntu::location::Criteria& rhs)
{
    Criteria result{lhs};

    result.requires.position |= rhs.requires.position;
    result.requires.velocity |= rhs.requires.velocity;
    result.requires.heading |= rhs.requires.heading;
    result.requires.altitude |= rhs.requires.altitude;

    if (rhs.accuracy.horizontal < result.accuracy.horizontal)
        result.accuracy.horizontal = rhs.accuracy.horizontal;

    if (result.accuracy.vertical)
    {
        if (rhs.accuracy.vertical && rhs.accuracy.vertical < result.accuracy.vertical)
        {
            result.accuracy.vertical = rhs.accuracy.vertical;
        }
    } else
    {
        result.accuracy.vertical = rhs.accuracy.vertical;
    }

    if (result.accuracy.velocity)
    {
        if (rhs.accuracy.velocity && rhs.accuracy.velocity < result.accuracy.velocity)
        {
            result.accuracy.velocity = rhs.accuracy.velocity;
        }
    } else
    {
        result.accuracy.velocity = rhs.accuracy.velocity;
    }

    if (result.accuracy.heading)
    {
        if (rhs.accuracy.heading && rhs.accuracy.heading < result.accuracy.heading)
        {
            result.accuracy.heading = rhs.accuracy.heading;
        }
    } else
    {
        result.accuracy.heading = rhs.accuracy.heading;
    }

    return result;
}
