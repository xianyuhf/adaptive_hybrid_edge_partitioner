cmake_minimum_required (VERSION 2.8)
project (edgepart)

set(CMAKE_VERBOSE_MAKEFILE off)

# find_package (glog REQUIRED)
# find_package (gflags REQUIRED)
# find_package (Boost REQUIRED)

# SET a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # SET the possible values of build type for cmake-gui
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11 -pthread -fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_EXE_LINK_FLAGS_RELEASE} -Ofast -DNDEBUG")


# add_subdirectory(threadpool11)
# include_directories(./threadpool11/include)


add_executable (main
    src/main.cpp
    src/util.cpp
    src/graph.cpp
    src/hep_partitioner.cpp
    src/ne_graph.cpp
    src/ne_partitioner.cpp
    src/Vertex2EdgePart.cpp
    src/conversions.cpp
    src/ahep_partitioner.cpp
    src/cluster.cpp)

# target_link_libraries (main glog gflags  threadpool11)
target_link_libraries (main glog gflags)


# rep (direction_u, 2) {
#   ne_adjlist_t &neighbors_u = direction_u ? adj_out[vid] : adj_in[vid];
#   for (size_t j = 0; j < neighbors_u.size();j++) {
#       if (edges[neighbors_u[j].v].valid()) {
#           vid_t &u_m = direction_u ? edges[neighbors_u[j].v].second : edges[neighbors_u[j].v].first;
#           if(!is_boundary.get(u_m)||!is_core.get(u_m)){
#               have_edge=true;
#               break;
#           }
#       }
#   }
#   if(have_edge) break;
# }