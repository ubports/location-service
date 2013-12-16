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
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <org/freedesktop/dbus/well_known_bus.h>

#include <functional>
#include <iostream>

namespace com {
namespace ubuntu {
namespace location {

struct ProgramOptions
{
    struct Errors
    {
        struct OptionNotSet {};
    };

    struct Options
    {
        static const char* bus() { return "bus"; }
    };

    ProgramOptions(bool do_allow_unregistered = true) : allow_unregistered(do_allow_unregistered)
    {
        add(Options::bus(),
            "The well-known bus to connect to the service upon",
            std::string{"session"});
    }

    ProgramOptions& add(const char* name, const char* desc)
    {
        od.add_options()
                (name, desc);
        return *this;
    }

    template<typename T>
    ProgramOptions& add(const char* name, const char* desc)
    {
        od.add_options()
                (name, boost::program_options::value<T>(), desc);
        return *this;
    }
    
    template<typename T>
    ProgramOptions& add(const char* name, const char* desc, const T& default_value)
    {
        od.add_options()
                (name, boost::program_options::value<T>()->default_value(default_value), desc);
        return *this;
    }

    template<typename T>
    ProgramOptions& add_composed(const char* name, const char* desc)
    {
        od.add_options()(
            name, 
            boost::program_options::value<T>()->composing(),
            desc);
        return *this;
    }

    bool parse_from_command_line_args(int argc, char** argv)
    {
        try
        {
            auto parser = boost::program_options::command_line_parser(
                argc, 
                argv).options(od);
            auto parsed = allow_unregistered ? parser.allow_unregistered().run() : parser.run();
            unrecognized = boost::program_options::collect_unrecognized(
                parsed.options, 
                boost::program_options::exclude_positional);
            boost::program_options::store(parsed, vm);
        } catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }

    org::freedesktop::dbus::WellKnownBus bus()
    {
        static const std::map<std::string, org::freedesktop::dbus::WellKnownBus> lut =
        {
            {"session", org::freedesktop::dbus::WellKnownBus::session},
            {"system", org::freedesktop::dbus::WellKnownBus::system},
        };

        return lut.at(value_for_key<std::string>(Options::bus()));
    }

    template<typename T>
    T value_for_key(const std::string& key)
    {
        return vm[key].as<T>();
    }

    std::size_t value_count_for_key(const std::string& key)
    {
        return vm.count(key);
    }

    void enumerate_unrecognized_options(const std::function<void(const std::string&)>& enumerator)
    {
        for (const std::string& s : unrecognized)
            enumerator(s);
    }

    void print_help(std::ostream& out)
    {
        out << od;
    }

    bool allow_unregistered;
    boost::program_options::options_description od;
    boost::program_options::variables_map vm;
    std::vector<std::string> unrecognized;
};

}
}
}
