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
#ifndef LOCATION_BOOST_PTREE_SETTINGS_H_
#define LOCATION_BOOST_PTREE_SETTINGS_H_

#include <location/settings.h>
#include <location/visibility.h>

#include <boost/property_tree/ptree.hpp>

namespace location
{
// Implements the settings interface relying on boost::property_tree.
// We rely on the INI file format for serializing data. While certainly
// dated and somewhat inefficient, it is human readable and easy to understand.
class LOCATION_DLL_PUBLIC BoostPtreeSettings : public Settings
{
public:
    // Creates a new instance, reading values from the given filename.
    BoostPtreeSettings(const std::string& fn);

    // Syncs the current settings to implementation-specific backends.
    void sync() override;

    // Returns true iff a value is known for the given key.
    bool has_value_for_key(const std::string& key) const override;

    // Gets a string value known for the given key, or throws Error::NoValueForKey.
    std::string get_string_for_key_or_throw(const std::string& key) override;

    // Sets values known for the given key.
    bool set_string_for_key(const std::string& key, const std::string& value) override;

private:
    // The filename we should read from/write to.
    std::string fn;
    // Stores values.
    boost::property_tree::ptree tree;
};
}

#endif // LOCATION_BOOST_PTREE_SETTINGS_H_
