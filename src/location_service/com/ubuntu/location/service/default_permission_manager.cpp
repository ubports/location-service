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
#include <com/ubuntu/location/service/default_permission_manager.h>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

culs::DefaultPermissionManager::DefaultPermissionManager()
    : pid(getpid()),
      uid(getuid())
{
}

culs::DefaultPermissionManager::~DefaultPermissionManager() noexcept
{
}

culs::PermissionManager::Result culs::DefaultPermissionManager::check_permission_for_credentials(
    const cul::Criteria&,
    const culs::Credentials& credentials)
{
    if (credentials.pid != pid || credentials.uid != uid)
        return Result::granted; // FIXME(tvoss): This should return rejected.
    return Result::granted;
}
