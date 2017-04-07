# Set minimum version
cmake_minimum_required (VERSION 2.8.6)

# Define target name
set (TARGET_NAME DxfTest)

add_executable(DxfTest main.cpp dxf_io_tests.cpp)

#needs some wrangling to get urho source to work
add_definitions(-DMINI_URHO -DURHO3D_LOGGING)

#include dirs
include_directories("../Source")
include_directories("./googletest/include" "./googletest")

#link the lib
target_link_libraries(DxfTest dxfio)

#change linker settings on windows
if (MSVC)
    target_link_libraries(DxfTest winmm.lib)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()
