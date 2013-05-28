#include "com/ubuntu/location/velocity.h"

#include <gtest/gtest.h>

TEST(Velocity, constructing_a_velocity_with_invalid_value_throws)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(
        -std::numeric_limits<double>::max(),
        com::ubuntu::location::Velocity::min().value() - std::numeric_limits<double>::min());
    double d = dist(rng);
    EXPECT_ANY_THROW(com::ubuntu::location::Velocity v {d*com::ubuntu::location::units::MetersPerSecond};);
}

TEST(Velocity, constructing_a_velocity_with_a_valid_value_does_not_throw)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Velocity::min().value(),
            com::ubuntu::location::Velocity::max().value());

    EXPECT_NO_THROW(com::ubuntu::location::Velocity v {dist(rng)*com::ubuntu::location::units::MetersPerSecond};);
}

TEST(Velocity, a_velocity_contains_value_passed_at_construction)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Velocity::min().value(),
            com::ubuntu::location::Velocity::max().value());
    double d = dist(rng);
    com::ubuntu::location::Velocity v {d* com::ubuntu::location::units::MetersPerSecond};
    EXPECT_EQ(d * com::ubuntu::location::units::MetersPerSecond, v.value);
}
