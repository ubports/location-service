#include "com/ubuntu/location/accuracy.h"

#include "com/ubuntu/location/heading.h"
#include "com/ubuntu/location/velocity.h"
#include "com/ubuntu/location/wgs84/altitude.h"
#include "com/ubuntu/location/wgs84/latitude.h"
#include "com/ubuntu/location/wgs84/longitude.h"

namespace cul = com::ubuntu::location;

TEST(HeadingAccuracy, classification_of_min_and_max_values_works_correctly)
{
    cul::Accuracy<cul::Heading> acc_max{cul::Heading{cul::Heading::max()}};
    EXPECT_EQ(cul::AccuracyLevel::worst = acc.classify());

    cul::Accuracy<cul::Heading> acc_min{cul::Heading{cul::Heading::min()}};
    EXPECT_EQ(cul::AccuracyLevel::best = acc.classify());
}
