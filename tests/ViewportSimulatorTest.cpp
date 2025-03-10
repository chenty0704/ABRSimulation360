#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportSimulator;

using namespace std;

TEST(ViewportSimulatorTest, BasicSimulation) {
    ViewportSimulator simulator({60., 1.}, 2);

    EXPECT_EQ(simulator.ToDistribution({0., -90.}), vector({
                  0.25, 0.25, 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., 90.}), vector({
                  0., 0., 0., 0.,
                  0.25, 0.25, 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({90., 0.}), vector({
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({-90., 0.}), vector({
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., -180.}), vector({
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0.25, 0.25,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., 0.}), vector({
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0.25, 0.25}));

    EXPECT_EQ(simulator.ToDistribution({0., -135.}), vector({
                  0.25, 0.25, 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0.25, 0.25,
                  0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., -45.}), vector({
                  0., 0., 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., 45.}), vector({
                  0., 0., 0., 0.,
                  0.25, 0.25, 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0.25, 0.25}));
    EXPECT_EQ(simulator.ToDistribution({0., 135.}), vector({
                  0., 0., 0., 0.,
                  0., 0., 0.25, 0.25,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0.25, 0.25, 0., 0.,
                  0., 0., 0., 0.,}));

    const vector<SphericalPosition> positions = {{0., -90.}, {0., 90.}};
    EXPECT_EQ(simulator.ToDistribution(positions), vector({
                  0.125, 0.125, 0.125, 0.125,
                  0.125, 0.125, 0.125, 0.125,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.,
                  0., 0., 0., 0.}));
}

TEST(ViewportSimulatorTest, WithoutTiling) {
    ViewportSimulator simulator({60., 1.}, 1);

    EXPECT_EQ(simulator.ToDistribution({0., -90.}), vector({1., 0., 0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., 90.}), vector({0., 1., 0., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({90., 0.}), vector({0., 0., 1., 0., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({-90., 0.}), vector({0., 0., 0., 1., 0., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., -180.}), vector({0., 0., 0., 0., 1., 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., 0.}), vector({0., 0., 0., 0., 0., 1.}));

    EXPECT_EQ(simulator.ToDistribution({0., -135.}), vector({0.5, 0., 0., 0., 0.5, 0.}));
    EXPECT_EQ(simulator.ToDistribution({0., -45.}), vector({0.5, 0., 0., 0., 0., 0.5}));
    EXPECT_EQ(simulator.ToDistribution({0., 45.}), vector({0., 0.5, 0., 0., 0., 0.5}));
    EXPECT_EQ(simulator.ToDistribution({0., 135.}), vector({0., 0.5, 0., 0., 0.5, 0.}));
}
