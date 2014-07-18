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
#include <com/ubuntu/location/providers/geoclue_provider.h>

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
