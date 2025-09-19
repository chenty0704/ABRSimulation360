#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.FlareAllocator;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

TEST(FlareAllocatorTest, BasicAllocation) {
    const StreamingConfig streamingConfig = {.BitratesPerFaceMbps = {1., 2., 4., 8.}, .TilingCount = 1};
    const vector predictedDistribution = {1., 0., 0., 0., 0., 0.};
    FlareAllocatorOptions options;
    options.InitialAccuracy = 0.;
    FlareAllocator allocator(streamingConfig, options);

    const auto DilatedDistribution = [&](double dilation) {
        return views::iota(0, 6) | views::transform([&](int tileID) {
            return predictedDistribution[tileID] * (1 - dilation) + dilation / 6;
        }) | ranges::to<vector>();
    };
    BitrateAllocatorContext context = {
        .AggregateBitrateMbps = 15.,
        .ViewportDistribution = predictedDistribution,
        .DilatedViewportDistribution = DilatedDistribution
    };
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({1, 1, 1, 1, 1, 1}));

    vector prevDistribution = {1., 0., 0., 0., 0., 0.};
    context.PrevViewportDistribution = prevDistribution;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({3, 0, 0, 0, 0, 0}));

    prevDistribution = {0., 1., 0., 0., 0., 0.};
    context.PrevViewportDistribution = prevDistribution;
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({2, 1, 1, 1, 1, 1}));
    EXPECT_EQ(allocator.GetBitrateIDs(context), vector({2, 1, 1, 1, 1, 1}));
}
