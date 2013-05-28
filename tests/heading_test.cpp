#include "com/ubuntu/location/heading.h"

#include <gtest/gtest.h>

TEST(Heading, constructing_a_heading_with_invalid_value_throws)
{
    EXPECT_ANY_THROW(com::ubuntu::location::Heading h {-std::numeric_limits<double>::min()*com::ubuntu::location::units::Degrees};);
    EXPECT_ANY_THROW(com::ubuntu::location::Heading h {361.*com::ubuntu::location::units::Degrees};);
}

TEST(Heading, constructing_a_heading_with_a_valid_value_does_not_throw)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Heading::min().value(),
            com::ubuntu::location::Heading::max().value());

    EXPECT_NO_THROW(com::ubuntu::location::Heading h {dist(rng)*com::ubuntu::location::units::Degrees};);
}

TEST(Heading, a_heading_contains_value_passed_at_construction)
{
    std::default_random_engine rng;
    std::uniform_real_distribution<double> dist(com::ubuntu::location::Heading::min().value(),
            com::ubuntu::location::Heading::max().value());
    double d = dist(rng);
    com::ubuntu::location::Heading h {d* com::ubuntu::location::units::Degrees};
    EXPECT_EQ(d * com::ubuntu::location::units::Degrees, h.value);
}
