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

#include <com/ubuntu/location/logging.h>

#include <boost/property_tree/ini_parser.hpp>

namespace location = com::ubuntu::location;

// Creates a new instance, reading values from the given filename.
location::BoostPtreeSettings::BoostPtreeSettings(const std::string& fn) : fn{fn}
{
    try
    {
        boost::property_tree::read_ini(fn, tree);
    }
    catch (boost::property_tree::ini_parser_error& e)
    {
        LOG(WARNING) << "Could not parse configuration file " << fn << ": " << e.what();
    }
}

// Syncs the current settings to implementation-specific backends.
void location::BoostPtreeSettings::sync()
{
    try
    {
        boost::property_tree::write_ini(fn, tree);
    }
    catch (const boost::property_tree::ini_parser_error& e)
    {
        LOG(WARNING) << "Could not store to configuration file " << fn << ": " << e.what();
    }
}

// Returns true iff a value is known for the given key.
bool location::BoostPtreeSettings::has_value_for_key(const std::string& key) const
{    
    return tree.count(key) > 0;
}

// Gets an integer value known for the given key, or throws Error::NoValueForKey.
std::string location::BoostPtreeSettings::get_string_for_key_or_throw(const std::string& key)
{
    try
    {
        return tree.get<std::string>(key);
    }
    catch (...)
    {
        throw location::Settings::Error::NoValueForKey{key};
    }
}

// Sets values known for the given key.
bool location::BoostPtreeSettings::set_string_for_key(const std::string& key, const std::string& value)
{
    bool result{false};
    try
    {
        tree.put(key, value);
        result = true;
    }
    catch (const boost::property_tree::ptree_bad_data&)
    {
        result = false;
    }

    return result;
}
