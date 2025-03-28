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
        "ViewportPredictors/NavGraphPredictor.ixx"
        "ViewportPredictors/StaticPredictor.ixx"
        "ViewportPredictors/ViewportPredictorFactory.ixx")
target_link_libraries(ViewportPredictors PUBLIC
        LibraryLinkUtilities::LibraryLinkUtilities
        System::System
        Base)

add_library(AggregateControllers)
target_sources(AggregateControllers PUBLIC FILE_SET CXX_MODULES FILES
        "AggregateControllers/AggregateControllerFactory.ixx"
        "AggregateControllers/IAggregateController.ixx"
        "AggregateControllers/ThroughputBasedController.ixx")
target_link_libraries(AggregateControllers PUBLIC
        System::System
        Base)

add_library(BitrateAllocators)
target_sources(BitrateAllocators PUBLIC FILE_SET CXX_MODULES FILES
        "BitrateAllocators/BitrateAllocatorFactory.ixx"
        "BitrateAllocators/HybridAllocator.ixx"
        "BitrateAllocators/IBitrateAllocator.ixx")
target_link_libraries(BitrateAllocators PUBLIC
        System::System
        Base)

add_library(ABRSimulation360)
target_sources(ABRSimulation360 PUBLIC FILE_SET CXX_MODULES FILES
        "NetworkSimulator.ixx"
        "ViewportPredictionSimulator.ixx"
        "ViewportSimulator.ixx")
target_link_libraries(ABRSimulation360
        PUBLIC
        System::System
        AggregateControllers
        Base
        BitrateAllocators
        ThroughputPredictors
        ViewportPredictors
        PRIVATE
        Microsoft::DirectXTK12)
