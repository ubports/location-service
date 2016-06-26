/*
 * Copyright © 2012-2015 Canonical Ltd.
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
 *              Manuel de la Pena <manuel.delapena@canonial.com>
 */

#include "provider.h"

namespace dbus = core::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    static dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

void location::providers::geoclue::Provider::start()
{
    if (!worker.joinable())
        worker = std::move(std::thread{std::bind(&dbus::Bus::run, bus)});
}

void location::providers::geoclue::Provider::stop()
{
    bus->stop();
    if (worker.joinable())
        worker.join();
}

void location::providers::geoclue::Provider::on_position_changed(const fd::Geoclue::Position::Signals::PositionChanged::ArgumentType& arg)
{
    fd::Geoclue::Position::FieldFlags flags{static_cast<unsigned long>(std::get<0>(arg))};
    location::Position pos
    {
        flags.test(fd::Geoclue::Position::Field::latitude) ?
            location::wgs84::Latitude{std::get<2>(arg)* location::units::Degrees} : location::wgs84::Latitude{},
        flags.test(fd::Geoclue::Position::Field::longitude) ?
            location::wgs84::Longitude{std::get<3>(arg)* location::units::Degrees} : location::wgs84::Longitude{}
    };

    if (flags.test(fd::Geoclue::Position::Field::altitude))
        pos.altitude = location::wgs84::Altitude{std::get<4>(arg)* location::units::Meters};

    location::Update<location::Position> update(pos);
    mutable_updates().position(update);
}

void location::providers::geoclue::Provider::on_velocity_changed(const fd::Geoclue::Velocity::Signals::VelocityChanged::ArgumentType& arg)
{
    fd::Geoclue::Velocity::FieldFlags flags{static_cast<unsigned long>(std::get<0>(arg))};
    if (flags.none())
        return;
    if (flags.test(fd::Geoclue::Velocity::Field::speed))
    {
        location::Update<location::Velocity> update
        {
            std::get<2>(arg) * location::units::MetersPerSecond,
            location::Clock::now()
        };
        mutable_updates().velocity(update);
    }

    if (flags.test(fd::Geoclue::Velocity::Field::direction))
    {
        location::Update<location::Heading> update
        {
            std::get<3>(arg) * location::units::Degrees,
            location::Clock::now()
        };

        mutable_updates().heading(update);
    }
}

location::Provider::Ptr location::providers::geoclue::Provider::create_instance(const location::ProviderFactory::Configuration& config)
{
    location::providers::geoclue::Provider::Configuration pConfig;
    pConfig.name = config.count(Configuration::key_name()) > 0 ?
                   config.get<std::string>(Configuration::key_name()) : throw std::runtime_error("Missing bus-name");
    pConfig.path = config.count(Configuration::key_path()) > 0 ?
                   config.get<std::string>(Configuration::key_path()) : throw std::runtime_error("Missing bus-path");
    return location::Provider::Ptr{new location::providers::geoclue::Provider{pConfig}};
}

location::providers::geoclue::Provider::Provider(const location::providers::geoclue::Provider::Configuration& config)
        : location::Provider(config.features, config.requirements),
          bus(the_session_bus()),
          service(dbus::Service::use_service(bus, config.name)),
          object(service->object_for_path(config.path)),
          signal_position_changed(object->get_signal<fd::Geoclue::Position::Signals::PositionChanged>()),
          signal_velocity_changed(object->get_signal<fd::Geoclue::Velocity::Signals::VelocityChanged>())
{
    position_updates_connection = signal_position_changed->connect(
            std::bind(&location::providers::geoclue::Provider::on_position_changed, this, std::placeholders::_1));
    velocity_updates_connection = signal_velocity_changed->connect(
            std::bind(&location::providers::geoclue::Provider::on_velocity_changed, this, std::placeholders::_1));

    auto info = object->invoke_method_synchronously<
        fd::Geoclue::GetProviderInfo,
        fd::Geoclue::GetProviderInfo::ResultType>();
    auto status = object->invoke_method_synchronously<
        fd::Geoclue::GetStatus,
        fd::Geoclue::GetStatus::ResultType>();
        
    std::cout << "GeoclueProvider: [" 
              << std::get<0>(info.value()) << ", " 
              << std::get<1>(info.value()) << ","
              << static_cast<fd::Geoclue::Status>(status.value()) << "]" <<std::endl;
}

location::providers::geoclue::Provider::~Provider() noexcept
{
    stop();
}

bool location::providers::geoclue::Provider::matches_criteria(const location::Criteria&)
{
    return true;
}

void location::providers::geoclue::Provider::start_position_updates()
{
    start();
}

void location::providers::geoclue::Provider::stop_position_updates()
{
    stop();
}

void location::providers::geoclue::Provider::start_velocity_updates()
{
    start();
}

void location::providers::geoclue::Provider::stop_velocity_updates()
{
    stop();
}    

void location::providers::geoclue::Provider::start_heading_updates()
{
    start();
}

void location::providers::geoclue::Provider::stop_heading_updates()
{
    stop();
}    
