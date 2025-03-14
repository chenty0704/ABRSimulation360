add_library(Base)
target_sources(Base PUBLIC FILE_SET CXX_MODULES FILES "Base.ixx")
target_link_libraries(Base PUBLIC
        LibraryLinkUtilities::LibraryLinkUtilities
        System::System)

add_library(ThroughputPredictors)
target_sources(ThroughputPredictors PUBLIC FILE_SET CXX_MODULES FILES
        "ThroughputPredictors/EMAPredictor.ixx"
        "ThroughputPredictors/IThroughputPredictor.ixx"
        "ThroughputPredictors/MovingAveragePredictor.ixx"
        "ThroughputPredictors/ThroughputPredictorFactory.ixx")
target_link_libraries(ThroughputPredictors PUBLIC
        System::System
        Base)

add_library(ViewportPredictors)
target_sources(ViewportPredictors PUBLIC FILE_SET CXX_MODULES FILES
        "ViewportPredictors/GravitationalPredictor.ixx"
        "ViewportPredictors/IViewportPredictor.ixx"
        "ViewportPredictors/LinearPredictor.ixx"
        "ViewportPredictors/StaticPredictor.ixx"
        "ViewportPredictors/ViewportPredictorFactory.ixx")
target_link_libraries(ViewportPredictors PUBLIC
        LibraryLinkUtilities::LibraryLinkUtilities
        System::System
        Base)

add_library(ABRSimulation360)
target_sources(ABRSimulation360 PUBLIC FILE_SET CXX_MODULES FILES
        "NetworkSimulator.ixx"
        "ViewportSimulator.ixx")
target_link_libraries(ABRSimulation360
        PUBLIC
        System::System
        Base
        PRIVATE
        Microsoft::DirectXTK12)
