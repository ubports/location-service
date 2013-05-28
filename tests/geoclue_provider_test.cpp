#include "com/ubuntu/location/providers/geoclue_provider.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(GeoclueProvider, accessing_ubuntu_geo_ip_works)
{
    const std::string name{"org.freedesktop.Geoclue.Providers.UbuntuGeoIP"};
    const std::string path{"/org/freedesktop/Geoclue/Providers/UbuntuGeoIP"};
    
    com::ubuntu::location::geoclue::Provider::Configuration config
    {
        name,
        path,
        com::ubuntu::location::geoclue::Provider::default_feature_flags(),
        com::ubuntu::location::geoclue::Provider::default_requirement_flags(),
    };

    com::ubuntu::location::geoclue::Provider provider(config);
    EXPECT_NO_THROW(provider.start_position_updates());
    EXPECT_NO_THROW(provider.stop_position_updates());
    EXPECT_NO_THROW(provider.start_velocity_updates());
    EXPECT_NO_THROW(provider.stop_velocity_updates());
    EXPECT_NO_THROW(provider.start_heading_updates());
    EXPECT_NO_THROW(provider.stop_heading_updates());
}
