add_executable(ABRSimulation360Test
        "NetworkSimulatorTest.cpp")
target_link_libraries(ABRSimulation360Test PRIVATE
        GTest::gtest_main
        ABRSimulation360)
