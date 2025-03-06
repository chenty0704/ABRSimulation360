cmake_minimum_required(VERSION 3.31)

project(ABRSimulation360
        VERSION 1.0
        DESCRIPTION "A research platform for 360° adaptive bitrate streaming"
        HOMEPAGE_URL "https://github.com/chenty0704/ABRSimulation360"
        LANGUAGES CXX)

find_package(LibraryLinkUtilities REQUIRED CONFIG)
find_package(System REQUIRED CONFIG)

add_subdirectory("modules")
add_subdirectory("src")
add_subdirectory("tests")
