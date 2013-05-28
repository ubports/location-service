#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_

#include <chrono>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Clock
{
    typedef std::chrono::high_resolution_clock::duration Duration;
    typedef std::chrono::high_resolution_clock::time_point Timestamp;

    static inline Timestamp now()
    {
        return std::chrono::high_resolution_clock::now();
    }
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CLOCK_H_
