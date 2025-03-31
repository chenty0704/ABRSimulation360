#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.BOLAAllocator;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

TEST(BOLAAllocatorTest, BasicAllocation) {
    const StreamingConfig streamingConfig = {1., {1., 2., 4., 8.}, 1, {60., 1.}, 5.};
    const vector distribution = {1., 0., 0., 0., 0., 0.};
    BOLAAllocator allocator(streamingConfig);

    BitrateAllocatorContext context = {.AggregateBitrateMbps = 15., .ViewportDistribution = distribution};
    context.BufferSeconds = 1.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({0, 0, 0, 0, 0, 0}));

    context.BufferSeconds = 2.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({1, 0, 0, 0, 0, 0}));

    context.BufferSeconds = 2.5;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({2, 0, 0, 0, 0, 0}));

    context.BufferSeconds = 3.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));
}
