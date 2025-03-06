add_library(Base)
target_sources(Base PUBLIC FILE_SET CXX_MODULES FILES "Base.ixx")
target_link_libraries(Base PUBLIC
        LibraryLinkUtilities::LibraryLinkUtilities
        System::System)

add_library(ABRSimulation360)
target_sources(ABRSimulation360 PUBLIC FILE_SET CXX_MODULES FILES
        "NetworkSimulator.ixx")
target_link_libraries(ABRSimulation360 PUBLIC
        System::System
        Base)
