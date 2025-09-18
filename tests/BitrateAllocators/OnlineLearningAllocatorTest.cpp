#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;
import ABRSimulation360.BitrateAllocators.OnlineLearningAllocator;

using namespace std;

TEST(OnlineLearningAllocatorTest, BasicAllocation) {
    const StreamingConfig streamingConfig = {.BitratesPerFaceMbps = {1., 2., 4., 8.}, .TilingCount = 1};
    const vector predictedDistribution = {1., 0., 0., 0., 0., 0.};
    OnlineLearningAllocatorOptions options;
    options.InitialTrustLevel = 0.;
    OnlineLearningAllocator allocator(streamingConfig, options);

    BitrateAllocatorContext context = {.AggregateBitrateMbps = 15., .ViewportDistribution = predictedDistribution};
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({1, 1, 1, 1, 1, 1}));

    vector prevDistribution = {1., 0., 0., 0., 0., 0.};
    context.PrevViewportDistribution = prevDistribution;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));

    prevDistribution = {0., 1., 0., 0., 0., 0.};
    context.PrevViewportDistribution = prevDistribution;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({1, 1, 1, 1, 1, 1}));
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({1, 1, 1, 1, 1, 1}));
}
