/*
 * Copyright © 2014 Canonical Ltd.
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

#include <com/ubuntu/location/boost_ptree_settings.h>

#include <gtest/gtest.h>

#include <cstdio>

namespace location = com::ubuntu::location;

namespace
{
const char* fn{"/tmp/totally_does_not_exist_file.ini"};
}

TEST(BoostPtreeSettings, does_not_throw_for_file_that_cannot_be_read_from)
{
    std::remove(fn);
    EXPECT_NO_THROW(location::BoostPtreeSettings settings{fn});
}

TEST(BoostPtreeSettings, never_throws_on_syncing)
{
    std::remove(fn);
    location::BoostPtreeSettings settings{fn};

    settings.set_string_for_key("test", "test");
    EXPECT_NO_THROW(settings.sync());
}

TEST(BoostPtreeSettings, values_are_correctly_stored)
{
    std::remove(fn);
    location::BoostPtreeSettings settings{fn};

    settings.set_string_for_key("test", "42");
    EXPECT_TRUE(settings.has_value_for_key("test"));
    EXPECT_EQ("42", settings.get_string_for_key_or_throw("test"));
}

TEST(BoostPtreeSettings, throws_correct_exception_for_missing_value)
{
    std::remove(fn);
    location::BoostPtreeSettings settings{fn};
    EXPECT_FALSE(settings.has_value_for_key("test"));
    EXPECT_THROW(settings.get_string_for_key_or_throw("test"), location::Settings::Error::NoValueForKey);
    EXPECT_EQ("42", settings.get_string_for_key("test", "42"));
}

TEST(BoostPtreeSettings, correctly_reads_and_writes_values_to_file)
{
    std::remove(fn);
    // We feed values in and sync them to file.
    {
        location::BoostPtreeSettings settings{fn};
        settings.set_string_for_key("test.test", "42");
        settings.sync();
    }

    // And read them back here.
    {
        location::BoostPtreeSettings settings{fn};
        EXPECT_EQ("42", settings.get_string_for_key_or_throw("test.test"));
    }
}
