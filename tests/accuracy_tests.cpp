#include "com/ubuntu/location/accuracy.h"

#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/velocity.h"
#include "com/ubuntu/location/wgs84/altitude.h"
#include "com/ubuntu/location/wgs84/latitude.h"
#include "com/ubuntu/location/wgs84/longitude.h"

#include <gtest/gtest.h>

namespace cul = com::ubuntu::location;

TEST(HeadingAccuracy, classification_of_min_and_max_values_works_correctly)
{
    cul::Accuracy<cul::Heading> acc_max{cul::Heading{cul::Heading::max()}};
    EXPECT_EQ(cul::AccuracyLevel::worst, acc_max.classify());

    cul::Accuracy<cul::Heading> acc_min{cul::Heading{cul::Heading::min()}};
    EXPECT_EQ(cul::AccuracyLevel::best, acc_min.classify());

    EXPECT_EQ(cul::AccuracyLevel::best, cul::Accuracy<cul::Heading>::best().classify());
    EXPECT_EQ(cul::AccuracyLevel::worst, cul::Accuracy<cul::Heading>::worst().classify());
}

TEST(VelocityAccuracy, classification_of_min_and_max_values_works_correctly)
{
    cul::Accuracy<cul::Velocity> acc_max{cul::Velocity{cul::Velocity::max()}};
    EXPECT_EQ(cul::AccuracyLevel::worst, acc_max.classify());

    cul::Accuracy<cul::Velocity> acc_min{cul::Velocity{cul::Velocity::min()}};
    EXPECT_EQ(cul::AccuracyLevel::best, acc_min.classify());

    EXPECT_EQ(cul::AccuracyLevel::best, cul::Accuracy<cul::Velocity>::best().classify());
    EXPECT_EQ(cul::AccuracyLevel::worst, cul::Accuracy<cul::Velocity>::worst().classify());
}
