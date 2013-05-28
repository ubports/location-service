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
#include "com/ubuntu/location/default_provider_set.h"

#include "com/ubuntu/location/provider.h"

#include <set>

namespace cul = com::ubuntu::location;

std::set<cul::Provider::Ptr> cul::default_provider_set(const cul::Provider::Ptr& seed)
{
    std::set<cul::Provider::Ptr> result{};
    if (seed)
        result.insert(seed);

    return result;
}
