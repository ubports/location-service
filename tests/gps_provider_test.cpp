#include "com/ubuntu/location/providers/gps.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(GeoclueProvider, accessing_starting_and_stopping_gps_provider_works)
{
    com::ubuntu::location::GpsProvider provider;
    EXPECT_NO_THROW(provider.start_position_updates());
    EXPECT_NO_THROW(provider.stop_position_updates());
    EXPECT_NO_THROW(provider.start_velocity_updates());
    EXPECT_NO_THROW(provider.stop_velocity_updates());
    EXPECT_NO_THROW(provider.start_heading_updates());
    EXPECT_NO_THROW(provider.stop_heading_updates());
}
