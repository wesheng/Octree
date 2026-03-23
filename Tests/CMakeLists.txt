
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG "v1.17.0"
)
set(gtest_force_shared_crt ON CACHE BOOL "" CMAKE_FASTBUILD_FORCE_RESPONSE_FILE)
FetchContent_MakeAvailable(googletest)

add_executable(OctreeTests "OctreeTests.cpp")
add_executable(Octrees::Tests ALIAS OctreeTests)
target_link_libraries(OctreeTests Octrees::Octree GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(OctreeTests)