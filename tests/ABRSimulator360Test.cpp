#include <gtest/gtest.h>

import System.Base;
import System.MDArray;

import ABRSimulation360.ABRSimulator360;
import ABRSimulation360.AggregateControllers.ThroughputBasedController;
import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.HybridAllocator;

using namespace std;
using namespace experimental;

TEST(ABRSimulator360Test, BasicSimulation) {
    const StreamingConfig streamingConfig = {1., {1., 2., 4., 8.}, 1, {60., 1.}, 5.};
    const vector throughputsMbps = {8., 32., 24., 16.};
    const NetworkSeriesView networkSeries = {1., throughputsMbps};
    const vector<SphericalPosition> positions(40);
    const ViewportSeriesView viewportSeries = {0.1, positions};

    double rebufferingSeconds;
    mdarray<double, dims<2>> bufferedBitratesMbps(4, 6);
    mdarray<double, dims<2>> distributions(4, 6);
    mdarray<double, dims<2>> predictedDistributions(3, 6);
    const SimulationSeriesRef simulationSeries = {
        rebufferingSeconds, bufferedBitratesMbps.to_mdspan(),
        distributions.to_mdspan(), predictedDistributions.to_mdspan()
    };
    ABRSimulator360::Simulate(streamingConfig, ThroughputBasedControllerOptions(), HybridAllocatorOptions(),
                              networkSeries, viewportSeries, simulationSeries);
    EXPECT_DOUBLE_EQ(rebufferingSeconds, 0.);
    EXPECT_EQ(bufferedBitratesMbps.container(), vector({
                  1., 1., 1., 1., 1., 1.,
                  1., 1., 1., 1., 1., 4.,
                  1., 1., 1., 1., 1., 4.,
                  1., 1., 1., 1., 1., 8.}));
    EXPECT_EQ(distributions.container(), vector({
                  0., 0., 0., 0., 0., 1.,
                  0., 0., 0., 0., 0., 1.,
                  0., 0., 0., 0., 0., 1.,
                  0., 0., 0., 0., 0., 1.}));
    EXPECT_EQ(predictedDistributions.container(), vector({
                  0., 0., 0., 0., 0., 1.,
                  0., 0., 0., 0., 0., 1.,
                  0., 0., 0., 0., 0., 1.}));
}
