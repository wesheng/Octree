add_library(Utility INTERFACE  "Vec3.h" "Bounds.h" "Mathf.h")

set_property(TARGET Utility PROPERTY CXX_STANDARD 20)

target_include_directories(Utility INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")