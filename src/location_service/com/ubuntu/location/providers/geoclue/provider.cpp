#include "com/ubuntu/location/providers/geoclue/provider.h"

#include "com/ubuntu/location/providers/geoclue/geoclue.h"

#include <thread>

namespace cul = com::ubuntu::location;
namespace culpg = com::ubuntu::location::providers::geoclue;

namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    static dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

struct culpg::Provider::Private
{
    Private(const culpg::Provider::Configuration& config) 
            : bus(the_session_bus()),
              service(dbus::Service::use_service(bus, config.name)),
              object(service->object_for_path(config.path)),
              signal_position_changed(object->get_signal<org::freedesktop::Geoclue::Position::Signals::PositionChanged>()),
              signal_velocity_changed(object->get_signal<org::freedesktop::Geoclue::Velocity::Signals::VelocityChanged>())  
    {        
    }
    
    void start() 
    { 
        if (!worker.joinable()) 
            worker = std::move(std::thread{std::bind(&dbus::Bus::run, bus)});
    }

    void stop() 
    { 
        bus->stop();
        if (worker.joinable())
            worker.join();
    }

    dbus::Bus::Ptr bus;
    dbus::Service::Ptr service;
    dbus::Object::Ptr object;
    dbus::Signal<
        org::freedesktop::Geoclue::Position::Signals::PositionChanged, 
        org::freedesktop::Geoclue::Position::Signals::PositionChanged::ArgumentType
        >::Ptr signal_position_changed;
    dbus::Signal<
        org::freedesktop::Geoclue::Velocity::Signals::VelocityChanged, 
        org::freedesktop::Geoclue::Velocity::Signals::VelocityChanged::ArgumentType
        >::Ptr signal_velocity_changed;
    dbus::signals::ScopedConnection position_updates_connection;
    dbus::signals::ScopedConnection velocity_updates_connection;

    std::thread worker;
};

cul::Provider::Ptr culpg::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culpg::Provider::Configuration pConfig;
    pConfig.name = config.count("name") > 0 ? config.at("name") : throw std::runtime_error("Missing bus-name");
    pConfig.path = config.count("path") > 0 ? config.at("path") : throw std::runtime_error("Missing bus-path");
    return cul::Provider::Ptr{new culpg::Provider{pConfig}};
}

const cul::Provider::FeatureFlags& culpg::Provider::default_feature_flags()
{
    static const cul::Provider::FeatureFlags flags{"001"};
    return flags;
}

const cul::Provider::RequirementFlags& culpg::Provider::default_requirement_flags()
{
    static const cul::Provider::RequirementFlags flags{"1010"};
    return flags;
}

culpg::Provider::Provider(const culpg::Provider::Configuration& config) 
        : com::ubuntu::location::Provider(config.features, config.requirements),
          d(new Private(config))
{
    d->position_updates_connection = 
            d->signal_position_changed->connect(
                [this](const org::freedesktop::Geoclue::Position::Signals::PositionChanged::ArgumentType& arg)
                {
                    org::freedesktop::Geoclue::Position::FieldFlags flags{static_cast<unsigned long>(std::get<0>(arg))};
                    cul::Update<cul::Position> update
                    {
                        {
                            flags.test(org::freedesktop::Geoclue::Position::Field::latitude) ? 
                                    cul::wgs84::Latitude{std::get<2>(arg)* cul::units::Degrees} : cul::wgs84::Latitude{},
                            flags.test(org::freedesktop::Geoclue::Position::Field::longitude) ? 
                                    cul::wgs84::Longitude{std::get<3>(arg)* cul::units::Degrees} : cul::wgs84::Longitude{},
                            flags.test(org::freedesktop::Geoclue::Position::Field::altitude) ? 
                                    cul::wgs84::Altitude{std::get<4>(arg)* cul::units::Meters} : cul::wgs84::Altitude{}
                        },
                        cul::Clock::now()
                    };
                    this->deliver_position_updates(update);
                });

    d->velocity_updates_connection = 
            d->signal_velocity_changed->connect(
                [this](const org::freedesktop::Geoclue::Velocity::Signals::VelocityChanged::ArgumentType& arg)
                {
                    org::freedesktop::Geoclue::Velocity::FieldFlags flags{static_cast<unsigned long>(std::get<0>(arg))};
                    if (flags.none())
                        return;
                    if (flags.test(org::freedesktop::Geoclue::Velocity::Field::speed))
                    {
                        cul::Update<cul::Velocity> update
                        {
                            std::get<2>(arg) * cul::units::MetersPerSecond,
                            cul::Clock::now()
                        };
                        this->deliver_velocity_updates(update);
                    }

                    if (flags.test(org::freedesktop::Geoclue::Velocity::Field::direction))
                    {
                        cul::Update<cul::Heading> update
                        {
                            std::get<3>(arg) * cul::units::Degrees,
                            cul::Clock::now()
                        };
                            
                        this->deliver_heading_updates(update);
                    }
                });

    auto info = d->object->invoke_method_synchronously<
        org::freedesktop::Geoclue::GetProviderInfo, 
        org::freedesktop::Geoclue::GetProviderInfo::ResultType>();
    auto status = d->object->invoke_method_synchronously<
        org::freedesktop::Geoclue::GetStatus, 
        org::freedesktop::Geoclue::GetStatus::ResultType>();
        
    std::cout << "GeoclueProvider: [" 
              << std::get<0>(info.value()) << ", " 
              << std::get<1>(info.value()) << ","
              << static_cast<org::freedesktop::Geoclue::Status>(status.value()) << "]" <<std::endl;
}

culpg::Provider::~Provider() noexcept
{
    d->stop();
}

bool culpg::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culpg::Provider::start_position_updates()
{
    d->start();
}

void culpg::Provider::stop_position_updates()
{
    d->stop();
}

void culpg::Provider::start_velocity_updates()
{
    d->start();
}

void culpg::Provider::stop_velocity_updates()
{
    d->stop();
}    

void culpg::Provider::start_heading_updates()
{
    d->start();
}

void culpg::Provider::stop_heading_updates()
{
    d->stop();
}    
