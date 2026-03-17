
add_library(Octree STATIC "Octree.h" "Octree.cpp")

set_property(TARGET Octree PROPERTY CXX_STANDARD 20)

target_include_directories(Octree PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")