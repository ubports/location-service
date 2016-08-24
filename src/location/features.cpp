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

#include <location/features.h>

#include <type_traits>

location::Features location::operator|(location::Features lhs, location::Features rhs)
{
    using NT = typename std::underlying_type<location::Features>::type;
    return static_cast<location::Features>(static_cast<NT>(lhs) | static_cast<NT>(rhs));
}

location::Features location::operator&(location::Features lhs, location::Features rhs)
{
    using NT = typename std::underlying_type<location::Features>::type;
    return static_cast<location::Features>(static_cast<NT>(lhs) & static_cast<NT>(rhs));
}
