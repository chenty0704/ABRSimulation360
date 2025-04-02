#include <gtest/gtest.h>

import System.Base;

import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.AggregateControllers.ModelPredictiveController;
import ABRSimulation360.Base;

using namespace std;

TEST(ModelPredictiveControllerTest, BasicControl) {
    const StreamingConfig streamingConfig = {1., {1., 2., 4., 8.}, 1, {60., 1.}, 5.};
    ModelPredictiveController controller(streamingConfig);

    AggregateControllerContext context;
    context.BufferSeconds = 2.;
    context.ThroughputMbps = 5.;
    EXPECT_DOUBLE_EQ(controller.GetAggregateBitrateMbps(context), 6.);

    context.ThroughputMbps = 15.;
    EXPECT_NEAR(controller.GetAggregateBitrateMbps(context), 8.5, 0.05);

    context.ThroughputMbps = 25.;
    EXPECT_DOUBLE_EQ(controller.GetAggregateBitrateMbps(context), 12.);

    context.BufferSeconds = 4.;
    context.ThroughputMbps = 5.;
    EXPECT_DOUBLE_EQ(controller.GetAggregateBitrateMbps(context), 6.);

    context.ThroughputMbps = 15.;
    EXPECT_NEAR(controller.GetAggregateBitrateMbps(context), 17.0, 0.05);

    context.ThroughputMbps = 25.;
    EXPECT_DOUBLE_EQ(controller.GetAggregateBitrateMbps(context), 24.);
}
