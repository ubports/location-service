#include "com/ubuntu/location/providers/skyhook/provider.h"

#include "com/ubuntu/location/logging.h"
#include "com/ubuntu/location/provider_factory.h"

#include <wpsapi.h>

#include <map>
#include <thread>

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::providers::skyhook;

namespace
{
static const std::map<int, std::string> return_code_lut =
{
    {WPS_OK, "WPS_OK"},
    {WPS_ERROR_SCANNER_NOT_FOUND, "WPS_ERROR_SCANNER_NOT_FOUND"},
    {WPS_ERROR_WIFI_NOT_AVAILABLE, "WPS_ERROR_WIFI_NOT_AVAILABLE"},
    {WPS_ERROR_NO_WIFI_IN_RANGE, "WPS_ERROR_NO_WIFI_IN_RANGE"},
    {WPS_ERROR_UNAUTHORIZED, "WPS_ERROR_UNAUTHORIZED"},
    {WPS_ERROR_SERVER_UNAVAILABLE, "WPS_ERROR_SERVER_UNAVAILABLE"},
    {WPS_ERROR_LOCATION_CANNOT_BE_DETERMINED, "WPS_ERROR_LOCATION_CANNOT_BE_DETERMINED"},
    {WPS_ERROR_PROXY_UNAUTHORIZED, "WPS_ERROR_PROXY_UNAUTHORIZED"},
    {WPS_ERROR_FILE_IO, "WPS_ERROR_FILE_IO"},
    {WPS_ERROR_INVALID_FILE_FORMAT, "WPS_ERROR_INVALID_FILE_FORMAT"},
    {WPS_ERROR_TIMEOUT, "WPS_ERROR_TIMEOUT"},
    {WPS_NOT_APPLICABLE, "WPS_NOT_APPLICABLE"},
    {WPS_GEOFENCE_ERROR, "WPS_GEOFENCE_ERROR"},
    {WPS_ERROR_NOT_TUNED, "WPS_ERROR_NOT_TUNED"},
    {WPS_NOMEM, "WPS_NOMEM"},
    {WPS_ERROR, "WPS_ERROR"}
};
}

struct culs::Provider::Private
{
    enum class State
    {
        stopped,
        started,
        stop_requested
    };

    static WPS_Continuation periodic_callback(
        void* context,
        WPS_ReturnCode code,
        const WPS_Location* location,
        const void*);
    
    Private(
        const culs::Provider::Configuration& config, 
        culs::Provider* parent)
            : parent(parent),
              config(config),
              state(State::stopped)
    {
    }
    
    void start()
    {
        if (state != State::stopped)
            return;
        
        if (worker.joinable())
            worker.join();

        static const unsigned infinite_iterations = 0;
        
        authentication.username = config.user_name.c_str();
        authentication.realm = config.realm.c_str();
        
        worker = std::move(std::thread([&]()
        {
            int rc = WPS_periodic_location(
                &authentication,
                WPS_NO_STREET_ADDRESS_LOOKUP,
                config.period.count(),
                infinite_iterations,
                culs::Provider::Private::periodic_callback,
                this);

            if (rc != WPS_OK)
                LOG(ERROR) << return_code_lut.at(rc);
        }));

        state = State::started;
    }
    
    void request_stop()
    {
        state = State::stop_requested;
    }

    culs::Provider* parent;
    Configuration config;
    State state;
    WPS_SimpleAuthentication authentication;
    std::thread worker;
};

WPS_Continuation culs::Provider::Private::periodic_callback(void* context,
                                                            WPS_ReturnCode code,
                                                            const WPS_Location* location,
                                                            const void*)
{
    if (code != WPS_OK)
    {
        LOG(WARNING) << return_code_lut.at(code);
        if (code == WPS_ERROR_WIFI_NOT_AVAILABLE)            
            return WPS_STOP;

        return WPS_CONTINUE;
    }

    auto thiz = static_cast<culs::Provider::Private*>(context);

    if (thiz->state == culs::Provider::Private::State::stop_requested)
    {
        LOG(INFO) << "Stop requested";
        thiz->state = culs::Provider::Private::State::stopped;
        return WPS_STOP;
    }

    cul::Position pos;
    pos.latitude(cul::wgs84::Latitude{location->latitude * cul::units::Degrees})
            .longitude(cul::wgs84::Longitude{location->longitude * cul::units::Degrees});
    if (location->altitude >= 0.f)
        pos.altitude(cul::wgs84::Altitude{location->altitude * cul::units::Meters});

    LOG(INFO) << pos;

    thiz->parent->deliver_position_updates(cul::Update<cul::Position>{pos, cul::Clock::now()});
        
    if (location->speed >= 0.f)
    {
        cul::Velocity v{location->speed * cul::units::MetersPerSecond};
        LOG(INFO) << v;
        thiz->parent->deliver_velocity_updates(cul::Update<cul::Velocity>{v, cul::Clock::now()});
    }

    if (location->bearing >= 0.f)
    {
        cul::Heading h{location->bearing * cul::units::Degrees};
        LOG(INFO) << h;
        thiz->parent->deliver_heading_updates(cul::Update<cul::Heading>{h, cul::Clock::now()});
    }

    return WPS_CONTINUE;
}
    
cul::Provider::Ptr culs::Provider::create_instance(const cul::ProviderFactory::Configuration& config)
{
    culs::Provider::Configuration configuration{"", "", std::chrono::milliseconds{100}};
    configuration.user_name = config.count("username") > 0 ? config.at("username") : "";
    configuration.realm = config.count("realm") > 0 ? config.at("realm") : "";
    if (config.count("period"))
    {
        int64_t value;
        std::stringstream ss(config.at("period"));
        ss >> value; configuration.period = std::chrono::milliseconds{value};
    }
    return cul::Provider::Ptr{new culs::Provider{configuration}};
}

const cul::Provider::FeatureFlags& culs::Provider::default_feature_flags()
{
    static const cul::Provider::FeatureFlags flags{"001"};
    return flags;
}

const cul::Provider::RequirementFlags& culs::Provider::default_requirement_flags()
{
    static const cul::Provider::RequirementFlags flags{"1010"};
    return flags;
}

culs::Provider::Provider(const culs::Provider::Configuration& config) 
        : com::ubuntu::location::Provider(culs::Provider::default_feature_flags(), culs::Provider::default_requirement_flags()),
          d(new Private(config, this))
{
}

culs::Provider::~Provider() noexcept
{
    d->request_stop();
}

bool culs::Provider::matches_criteria(const cul::Criteria&)
{
    return true;
}

void culs::Provider::start_position_updates()
{
    d->start();
}

void culs::Provider::stop_position_updates()
{
    d->request_stop();
}

void culs::Provider::start_velocity_updates()
{
    d->start();
}

void culs::Provider::stop_velocity_updates()
{
    d->request_stop();
}    

void culs::Provider::start_heading_updates()
{
    d->start();
}

void culs::Provider::stop_heading_updates()
{
    d->request_stop();
}
