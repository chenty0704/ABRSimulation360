#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.HybridAllocator;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

TEST(HybridAllocatorTest, BasicAllocation) {
    const StreamingConfig streamingConfig = {.BitratesPerFaceMbps = {1., 2., 4., 8.}, .TilingCount = 1};
    const vector distribution = {1., 0., 0., 0., 0., 0.};

    HybridAllocatorOptions options;
    options.TrustLevel = 0.;
    HybridAllocator allocator(streamingConfig, options);
    BitrateAllocatorContext context = {.ViewportDistribution = distribution};
    context.AggregateBitrateMbps = 5.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({0, 0, 0, 0, 0, 0}));

    context.AggregateBitrateMbps = 15.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({2, 1, 1, 1, 1, 1}));

    context.AggregateBitrateMbps = 25.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({2, 2, 2, 2, 2, 2}));

    options.TrustLevel = 0.5;
    allocator = HybridAllocator(streamingConfig, options);
    context.AggregateBitrateMbps = 5.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({0, 0, 0, 0, 0, 0}));

    context.AggregateBitrateMbps = 15.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 1, 1, 0, 0, 0}));

    context.AggregateBitrateMbps = 25.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 2, 2, 2, 1, 1}));

    options.TrustLevel = 1.;
    allocator = HybridAllocator(streamingConfig, options);
    context.AggregateBitrateMbps = 5.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({0, 0, 0, 0, 0, 0}));

    context.AggregateBitrateMbps = 15.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));

    context.AggregateBitrateMbps = 25.;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));
}
