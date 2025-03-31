add_executable(ThroughputPredictorsTest
        "ThroughputPredictors/EMAPredictorTest.cpp"
        "ThroughputPredictors/MovingAveragePredictorTest.cpp")
target_link_libraries(ThroughputPredictorsTest PRIVATE
        GTest::gtest_main
        ThroughputPredictors)

add_executable(ViewportPredictorsTest
        "ViewportPredictors/LinearPredictorTest.cpp")
target_link_libraries(ViewportPredictorsTest PRIVATE
        GTest::gtest_main
        ViewportPredictors)

add_executable(BitrateAllocatorsTest
        "BitrateAllocators/BOLAAllocatorTest.cpp"
        "BitrateAllocators/HybridAllocatorTest.cpp"
        "BitrateAllocators/OnlineLearningAllocatorTest.cpp")
target_link_libraries(BitrateAllocatorsTest PRIVATE
        GTest::gtest_main
        BitrateAllocators)

add_executable(ABRSimulation360Test
        "ABRSimulator360Test.cpp"
        "NetworkSimulatorTest.cpp"
        "ViewportPredictionSimulatorTest.cpp"
        "ViewportSimulatorTest.cpp")
target_link_libraries(ABRSimulation360Test PRIVATE
        GTest::gtest_main
        ABRSimulation360)
