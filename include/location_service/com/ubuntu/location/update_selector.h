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
#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_SELECTOR_H
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_SELECTOR_H

#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/position.h>

namespace com
{
namespace ubuntu
{
namespace location
{

template<typename T>
struct WithSource { std::shared_ptr<Provider> source; T value; };

class UpdateSelector
{
public:
    typedef std::shared_ptr<UpdateSelector> Ptr;

    UpdateSelector(const UpdateSelector&) = delete;
    UpdateSelector& operator=(const UpdateSelector&) = delete;
    virtual ~UpdateSelector() = default;

    virtual WithSource<Update<Position>> select(const WithSource<Update<Position>>& older,
                                                const WithSource<Update<Position>>& newer) = 0;

protected:
    UpdateSelector() = default;
};
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_SELECTOR_H

