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

void dummy::Provider::add_to_registry()
{
    ProviderRegistry::instance().add_provider_for_name("dummy::Provider", [](const util::settings::Source& settings)
    {
        return dummy::Provider::create_instance(settings);
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

location::Provider::Ptr dummy::Provider::create_instance(const util::settings::Source& settings)
{
    dummy::Configuration provider_config;

    provider_config.update_period = std::chrono::milliseconds
    {
        settings.get_value<std::uint64_t>(Configuration::Keys::update_period, 500)
    };

    provider_config.reference_position
            .latitude(settings.get_value<float>(Configuration::Keys::reference_position_lat, 51.) * location::units::degrees)
            .longitude(settings.get_value<float>(Configuration::Keys::reference_position_lon,7.) * location::units::degrees);
    provider_config.reference_position
            .altitude(settings.get_value<float>(Configuration::Keys::reference_position_alt, 0.) * location::units::meters);
    provider_config.reference_position.accuracy()
            .horizontal(settings.get_value<float>(Configuration::Keys::reference_horizontal_accuracy, 0.) * location::units::meters);
    provider_config.reference_position.accuracy()
            .vertical(settings.get_value<float>(dummy::Configuration::Keys::reference_vertical_accuracy, 0.) * location::units::meters);

    provider_config.reference_velocity =
            settings.get_value<float>(Configuration::Keys::reference_velocity, 9.) * location::units::meters_per_second;
    provider_config.reference_heading =
            settings.get_value<float>(Configuration::Keys::reference_heading, 127.) * location::units::degrees;

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
