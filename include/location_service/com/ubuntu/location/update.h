#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_

#include "com/ubuntu/location/clock.h"

#include <ostream>

namespace com
{
namespace ubuntu
{
namespace location
{
template<typename T>
struct Update
{
    bool operator==(const Update<T>& rhs) const
    {
        return value == rhs.value && when == rhs.when;
    }

    bool operator!=(const Update<T>& rhs) const
    {
        return !(value == rhs.value && when == rhs.when);
    }

    T value;
    Clock::Timestamp when;
};

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const Update<T>& update)
{
    out << "Update(" << update.value << ", " << update.when.time_since_epoch().count() << ")";
    return out;
}

}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_UPDATE_H_
