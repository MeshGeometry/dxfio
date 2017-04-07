cmake_minimum_required (VERSION 2.8.6)

project(dxfio C CXX)

if (UNIX)
  set (CMAKE_CXX_STANDARD 11)
endif ()

add_subdirectory("./Source")
add_subdirectory("./Test")