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
#include <location/service/default_permission_manager.h>

#include <location/criteria.h>

#include <gtest/gtest.h>

TEST(DefaultPermissionManager, for_now_only_local_sessions_are_accepted)
{
    //location::Credentials invalid_credentials{0, 0};
    location::service::Credentials valid_credentials{getpid(), getuid()};
    location::service::DefaultPermissionManager pm;
    /*EXPECT_EQ(location::DefaultPermissionManager::Result::rejected,
              pm.check_permission_for_credentials(location::Criteria {},
              invalid_credentials));*/
    EXPECT_EQ(location::service::PermissionManager::Result::granted,
              pm.check_permission_for_credentials(location::Criteria {},
                                                  valid_credentials));
}
