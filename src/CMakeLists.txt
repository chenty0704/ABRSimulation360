add_library(ABRSimulation360Link SHARED "ABRSimulation360Link.cpp")
target_link_libraries(ABRSimulation360Link PRIVATE
        LibraryLinkUtilities::LibraryLinkUtilities
        System::System
        ABRSimulation360)
