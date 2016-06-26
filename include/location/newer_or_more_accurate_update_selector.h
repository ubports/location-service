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
#ifndef LOCATION_NEWER_OR_MORE_ACCURATE_UPDATE_SELECTOR_H
#define LOCATION_NEWER_OR_MORE_ACCURATE_UPDATE_SELECTOR_H

#include <location/update_selector.h>

namespace location
{
class NewerOrMoreAccurateUpdateSelector : public UpdateSelector
{
public:
    typedef std::shared_ptr<NewerOrMoreAccurateUpdateSelector> Ptr;

    WithSource<Update<Position>> select(const WithSource<Update<Position>>& older,
                                        const WithSource<Update<Position>>& newer) override
    {
        // Basically copied this value from the Android fusion provider
        static const std::chrono::seconds cutoff(11);

        // If the new position is from the same source as the old one then just use it
        if (newer.source == older.source) {
            return newer;
        }

        // If the new position is newer by a significant margin then just use it
        if (newer.value.when > older.value.when + cutoff) {
            return newer;
        }

        // Choose the position with the smaller accuracy circle if both have them
        if (!older.value.value.accuracy.horizontal)
            return newer;
        if (!newer.value.value.accuracy.horizontal)
            return older;
        if (newer.value.value.accuracy.horizontal < older.value.value.accuracy.horizontal)
            return newer;
        else
            return older;
    }
};
}

#endif // LOCATION_NEWER_OR_MORE_ACCURATE_UPDATE_SELECTOR_H

