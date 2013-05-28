#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_

#include "com/ubuntu/location/accuracy.h"
#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/velocity.h"
#include "com/ubuntu/location/wgs84/altitude.h"
#include "com/ubuntu/location/wgs84/latitude.h"
#include "com/ubuntu/location/wgs84/longitude.h"

#include <limits>
#include <ostream>
#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Criteria
{
    Criteria() : latitude_accuracy(),
                 longitude_accuracy(),
                 altitude_accuracy(),
                 velocity_accuracy(),
                 heading_accuracy()
    {
    }
    
    Accuracy<wgs84::Latitude> latitude_accuracy;
    Accuracy<wgs84::Longitude> longitude_accuracy;
    Accuracy<wgs84::Altitude> altitude_accuracy;
    Accuracy<Velocity> velocity_accuracy;
    Accuracy<Heading> heading_accuracy;
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CRITERIA_H_
