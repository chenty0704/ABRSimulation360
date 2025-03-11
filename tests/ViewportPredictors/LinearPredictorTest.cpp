#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.LinearPredictor;

using namespace std;

TEST(LinearPredictorTest, BasicPrediction) {
    LinearPredictorOptions options;
    options.HistorySeconds = 4.;
    LinearPredictor predictor(1., options);

    vector<SphericalPosition> positions = {{0., 0.}, {10., 10.}};
    predictor.Update(positions);
    EXPECT_EQ(predictor.PredictPositions(0., 2.), vector<SphericalPosition>({{20., 20.}, {30., 30.}}));

    positions = {{10., 10.}, {0., 0.}};
    predictor.Update(positions);
    EXPECT_EQ(predictor.PredictPositions(0., 2.), vector<SphericalPosition>({{5., 5.}, {5., 5.}}));

    positions = {{0., 50.}, {0., 100.}, {0., 150.}, {0., -160.}};
    predictor.Update(positions);
    EXPECT_EQ(predictor.PredictPositions(0., 2.), vector<SphericalPosition>({{0., -110.}, {0., -60.}}));
}
