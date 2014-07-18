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

#include <com/ubuntu/location/criteria.h>

#include <gtest/gtest.h>

TEST(DefaultPermissionManager, for_now_only_local_sessions_are_accepted)
{
    //com::ubuntu::location::Credentials invalid_credentials{0, 0};
    com::ubuntu::location::service::Credentials valid_credentials{getpid(), getuid()};
    com::ubuntu::location::service::DefaultPermissionManager pm;
    /*EXPECT_EQ(com::ubuntu::location::DefaultPermissionManager::Result::rejected,
              pm.check_permission_for_credentials(com::ubuntu::location::Criteria {},
              invalid_credentials));*/
    EXPECT_EQ(com::ubuntu::location::service::PermissionManager::Result::granted,
              pm.check_permission_for_credentials(com::ubuntu::location::Criteria {},
                                                  valid_credentials));
}
