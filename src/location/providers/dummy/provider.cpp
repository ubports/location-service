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

#include "provider.h"

#include <location/logging.h>
#include <location/glib/runtime.h>

#include <core/posix/this_process.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <thread>

namespace dummy = location::providers::dummy;
namespace env = core::posix::this_process::env;
namespace fs = boost::filesystem;

namespace
{

struct SettingsHelper
{
    template<typename T>
    static T get_value(std::string key, T&& default_value)
    {
        static const std::string snap_path = env::get("SNAP_DATA");

        boost::filesystem::path path{snap_path};
        std::string key_path{key};
        std::replace_copy(key.begin(), key.end(), key_path.begin(), '.', '/');
        path /= key_path;

        LOG(INFO) << "Reading setting from " << path.string();

        T value{default_value};

        if (fs::exists(path))
        {
            std::ifstream in{path.string().c_str()};
            in >> value;
        }

        return value;
    }
};

}  // namespace

void dummy::Provider::add_to_registry()
{
    ProviderRegistry::instance().add_provider_for_name("dummy::Provider", [](const ProviderRegistry::Configuration& configuration)
    {
        return dummy::Provider::create_instance(configuration);
    },
    {
        {dummy::Configuration::Keys::reference_position_lat, "latitude reported by the provider instance"},
        {dummy::Configuration::Keys::reference_position_lon, "longitude reported by the provider instance"},
        {dummy::Configuration::Keys::reference_position_alt, "altitude reported by the provider instance"},
        {dummy::Configuration::Keys::reference_velocity, "velocity reported by the provider instance"},
        {dummy::Configuration::Keys::reference_heading, "heading reported by the provider instance"},
        {dummy::Configuration::Keys::reference_horizontal_accuracy, "horizontal accuracy reported by the provider instance"},
        {dummy::Configuration::Keys::reference_vertical_accuracy, "vertical accuracy reported by the provider instance"},
        {dummy::Configuration::Keys::update_period, "updates are emitted at this rate"},
    });
}

location::Provider::Ptr dummy::Provider::create_instance(const location::ProviderRegistry::Configuration& config)
{
    dummy::Configuration provider_config;

    provider_config.update_period = std::chrono::milliseconds
    {
        config.get(Configuration::Keys::update_period, SettingsHelper::get_value(Configuration::Keys::update_period, 500))
    };
    provider_config.reference_position
            .latitude(config.get(Configuration::Keys::reference_position_lat, SettingsHelper::get_value(Configuration::Keys::reference_position_lat, 51.)) * location::units::degrees)
            .longitude(config.get(dummy::Configuration::Keys::reference_position_lon, SettingsHelper::get_value(Configuration::Keys::reference_position_lon, 7.)) * location::units::degrees);

    if (config.count(dummy::Configuration::Keys::reference_position_alt) > 0)
        provider_config.reference_position.altitude(config.get(dummy::Configuration::Keys::reference_position_alt, 0.) * location::units::meters);

    if (config.count(dummy::Configuration::Keys::reference_horizontal_accuracy) > 0)
        provider_config.reference_position.accuracy().horizontal(config.get(dummy::Configuration::Keys::reference_horizontal_accuracy, 0.) * location::units::meters);

    if (config.count(dummy::Configuration::Keys::reference_vertical_accuracy) > 0)
        provider_config.reference_position.accuracy().vertical(config.get(dummy::Configuration::Keys::reference_vertical_accuracy, 0.) * location::units::meters);

    provider_config.reference_velocity = config.get(dummy::Configuration::Keys::reference_velocity, SettingsHelper::get_value(Configuration::Keys::reference_velocity, 9.)) * location::units::meters_per_second;
    provider_config.reference_heading = config.get(dummy::Configuration::Keys::reference_heading, SettingsHelper::get_value(Configuration::Keys::reference_heading, 127.)) * location::units::degrees;

    return location::Provider::Ptr{new dummy::Provider{provider_config}};
}

dummy::Provider::Provider(const dummy::Configuration& configuration)
    : configuration{configuration}
{
}

dummy::Provider::~Provider() noexcept
{
    deactivate();

    if (worker.joinable())
        worker.join();
}

void dummy::Provider::on_new_event(const Event&)
{
}

location::Provider::Requirements dummy::Provider::requirements() const
{
    return Requirements::none;
}

bool dummy::Provider::satisfies(const location::Criteria&)
{
    return true;
}

void dummy::Provider::enable()
{
}

void dummy::Provider::disable()
{
}

void dummy::Provider::activate()
{
    stop_requested = false;

    worker = std::move(std::thread([this]()
    {
        VLOG(1) << __PRETTY_FUNCTION__ << ": started";

        location::Update<location::Position> position_update
        {
            configuration.reference_position,
            location::Clock::now()
        };

        location::Update<location::units::Degrees> heading_update
        {
            configuration.reference_heading,
            location::Clock::now()
        };

        location::Update<location::units::MetersPerSecond> velocity_update
        {
            configuration.reference_velocity,
            location::Clock::now()
        };

        while (!stop_requested)
        {
            position_update.when = location::Clock::now();
            heading_update.when = location::Clock::now();
            velocity_update.when = location::Clock::now();

            glib::Runtime::instance()->dispatch([this, pu = position_update, hu = heading_update, vu = velocity_update]()
            {
                updates.position(pu);
                updates.heading(hu);
                updates.velocity(vu);
            });

            std::this_thread::sleep_for(configuration.update_period);
        }
    }));
}

void dummy::Provider::deactivate()
{
    VLOG(1) << __PRETTY_FUNCTION__ << ": stopping";

    stop_requested = true;

    if (worker.joinable())
        worker.join();

    VLOG(1) << __PRETTY_FUNCTION__ << ": stopping";
}

const core::Signal<location::Update<location::Position>>& dummy::Provider::position_updates() const
{
    return updates.position;
}

const core::Signal<location::Update<location::units::Degrees>>& dummy::Provider::heading_updates() const
{
    return updates.heading;
}

const core::Signal<location::Update<location::units::MetersPerSecond>>& dummy::Provider::velocity_updates() const
{
    return updates.velocity;
}
