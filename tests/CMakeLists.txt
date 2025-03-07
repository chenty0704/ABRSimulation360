add_executable(ThroughputPredictorsTest
        "ThroughputPredictors/EMAPredictorTest.cpp"
        "ThroughputPredictors/MovingAveragePredictorTest.cpp")
target_link_libraries(ThroughputPredictorsTest PRIVATE
        GTest::gtest_main
        ThroughputPredictors)

add_executable(ABRSimulation360Test
        "NetworkSimulatorTest.cpp")
target_link_libraries(ABRSimulation360Test PRIVATE
        GTest::gtest_main
        ABRSimulation360)
