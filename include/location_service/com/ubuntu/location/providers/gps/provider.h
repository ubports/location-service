#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_PROVIDER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_PROVIDER_H_

#include "com/ubuntu/location/provider.h"
#include "com/ubuntu/location/provider_factory.h"

namespace com
{
namespace ubuntu
{
namespace location
{
namespace providers
{
namespace gps
{
class Provider : public com::ubuntu::location::Provider
{
  public:
    static std::string class_name();
    static Provider::Ptr create_instance(const ProviderFactory::Configuration&);

    static const Provider::FeatureFlags& default_feature_flags();
    static const Provider::RequirementFlags& default_requirement_flags();

    Provider();
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;
    ~Provider() noexcept;

    virtual bool matches_criteria(const Criteria&);

    virtual void start_position_updates();
    virtual void stop_position_updates();

    virtual void start_velocity_updates();
    virtual void stop_velocity_updates();

    virtual void start_heading_updates();
    virtual void stop_heading_updates();

  private:
    struct Private;
    std::unique_ptr<Private> d;    
};
}
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_PROVIDERS_GPS_PROVIDER_H_
