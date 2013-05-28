#include "com/ubuntu/location/service/default_permission_manager.h"

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
