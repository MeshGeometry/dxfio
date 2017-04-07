cmake_minimum_required (VERSION 2.8.6)

if (UNIX)
    find_package (Threads REQUIRED)
endif ()

file(GLOB CORE_SRC
    "./Container/*.h"
    "./Container/*.cpp"
    "./Core/*.h"
    "./Core/*.cpp"
    "./IO/*.h"
    "./IO/*.cpp"
    "./Math/*.h"
    "./Math/*.cpp"
    "./LZ4/*.h"
    "./LZ4/*.c"
    )

file(GLOB DXFIO_SRC
    "./Dxf/*.h"
    "./Dxf/*.cpp"
	)


include_directories("./")

add_library(dxfio ${CORE_SRC} ${DXFIO_SRC})

source_group("Common" FILES ${CORE_SRC})
source_group("Dxf" FILES ${DXFIO_SRC})

add_definitions(-DMINI_URHO -DURHO3D_LOGGING)
set_target_properties(dxfio PROPERTIES LINKER_LANGUAGE CXX)

if (UNIX)
    target_link_libraries (dxfio ${CMAKE_THREAD_LIBS_INIT})
endif ()