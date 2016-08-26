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
 * Authored by: Scott Sweeny <scott.sweeny@canonical.com>
 */
#ifndef LOCATION_ALWAYS_GRANTING_PERMISSION_MANAGER_H_
#define LOCATION_ALWAYS_GRANTING_PERMISSION_MANAGER_H_

#include <location/permission_manager.h>

namespace core { namespace dbus {
class Bus;
}}

namespace location
{
// A PermissionManager implementation which always returns granted since 
// permissions are handed by interfaces in Snappy
class AlwaysGrantingPermissionManager : public PermissionManager
{
public:
    AlwaysGrantingPermissionManager() = default;

    // From PermissionManager
    Result check_permission_for_credentials(const Criteria&, const Credentials&) override
    {
        return Result::granted;
    }
};
}

#endif // LOCATION_ALWAYS_GRANTING_PERMISSION_MANAGER_H_
