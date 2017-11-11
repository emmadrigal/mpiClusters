cmake_minimum_required(VERSION 3.1)

project(Cluster)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include_directories(${MPI_INCLUDE_PATH})


add_executable(Cluster src/median.cpp)

find_package(MPI REQUIRED)
find_package( OpenCV REQUIRED )
find_package( Boost COMPONENTS program_options REQUIRED )

set_property(TARGET Cluster PROPERTY CXX_STANDARD 11)

target_link_libraries(Cluster ${OpenCV_LIBS}  ${Boost_LIBRARIES} ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(Cluster PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(Cluster PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()