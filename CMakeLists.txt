cmake_minimum_required(VERSION 3.1)
project(CLUSTER)

find_package(MPI REQUIRED)
find_package( OpenCV REQUIRED )

include_directories(${MPI_INCLUDE_PATH})

add_executable(median src/median.cpp)
target_link_libraries(median  ${OpenCV_LIBS} ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(median PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(median PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

