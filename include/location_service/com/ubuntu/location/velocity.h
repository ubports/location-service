#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_VELOCITY_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_VELOCITY_H_

#include "com/ubuntu/location/accuracy.h"
#include "com/ubuntu/location/units/units.h"

#include <limits>
#include <ostream>
#include <stdexcept>

namespace com
{
namespace ubuntu
{
namespace location
{
struct Velocity
{
    typedef units::Velocity Unit;
    typedef units::Quantity<Unit> Quantity;

    static inline const Quantity& min()
    {
        static const Quantity instance = Quantity::from_value(0.);
        return instance;
    }

    static inline const Quantity max()
    {
        static const Quantity instance = Quantity::from_value(std::numeric_limits<double>::max());
        return instance;
    }

    Velocity(const Quantity& value = Quantity()) : value(value)
    {
        if (value < Velocity::min())
            throw std::out_of_range("");
        if (value > Velocity::max())
            throw std::out_of_range("");
    }

    inline bool operator==(const Velocity& rhs) const
    {
        return value == rhs.value;
    }

    inline bool operator!=(const Velocity& rhs) const
    {
        return value != rhs.value;
    }

    Quantity value;
};

inline std::ostream& operator<<(std::ostream& out, const Velocity& velocity)
{
    out << "Velocity(" << velocity.value << ")";
    return out;
}

template<>
struct AccuracyTraits<Velocity>
{
    static AccuracyLevel classify(const Velocity& h)
    {
        if (h.value > (1.f * units::MetersPerSecond))
            return AccuracyLevel::worst;
        
        if (h.value <= (1.f * units::MetersPerSecond))
            return AccuracyLevel::best;

        return AccuracyLevel::worst;
    }

    static Accuracy<Velocity> best()
    {
        return Accuracy<Velocity>{Velocity{Velocity::min()}};
    }

    static Accuracy<Velocity> worst()
    {
        return Accuracy<Velocity>{Velocity{2*units::MetersPerSecond}};
    }
};
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_VELOCITY_H_
