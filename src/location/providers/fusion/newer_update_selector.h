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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com
 */
#ifndef LOCATION_ALWAYS_SELECT_NEW_UPDATE_H
#define LOCATION_ALWAYS_SELECT_NEW_UPDATE_H

#include <location/update_selector.h>

namespace location
{
class NewerUpdateSelector : public UpdateSelector
{
public:
    typedef std::shared_ptr<NewerUpdateSelector> Ptr;

    virtual WithSource<Update<Position>> select(const WithSource<Update<Position>>&,
                                                const WithSource<Update<Position>>& newer) override
    {
        return newer;
    }
};
}

#endif // LOCATION_ALWAYS_SELECT_NEW_UPDATE_H

