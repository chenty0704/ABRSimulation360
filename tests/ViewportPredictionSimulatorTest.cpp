#include <gtest/gtest.h>

import System.Base;
import System.MDArray;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictionSimulator;
import ABRSimulation360.ViewportPredictors.StaticPredictor;

using namespace std;
using namespace experimental;

TEST(ViewportPredictionSimulatorTest, BasicSimulation) {
    const vector<SphericalPosition> positions = {{0., 0.}, {10., 10.}, {20., 20.}, {30., 30.}};
    const ViewportSeriesView viewportSeries = {1., positions};

    mdarray<SphericalPosition, dims<2>> predictedPositions(2, 2);
    ViewportPredictionSimulator::Simulate(2, 1., StaticPredictorOptions(), viewportSeries,
                                          predictedPositions.to_mdspan());
    EXPECT_EQ(predictedPositions.container(), vector<SphericalPosition>({
                  {10., 10.}, {20., 20.},
                  {0., 0.}, {10., 10.}}));
}
