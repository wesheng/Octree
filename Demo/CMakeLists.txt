# CMakeList.txt : CMake project for Octree, include source and define
# project specific logic here.
#
include(FetchContent)

# Add source to this project's executable.
add_executable (Demo "App.cpp" "App.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Octree PROPERTY CXX_STANDARD 20)
endif()


FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG "3.4"
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG "1.0.3"
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
	glad
	GIT_REPOSITORY https://github.com/Dav1dde/glad.git
	GIT_TAG "v2.0.8"
	SOURCE_SUBDIR "cmake"
)
FetchContent_MakeAvailable(glad)
GLAD_ADD_LIBRARY(glad_gl REPRODUCIBLE API gl:core=4.6)

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG "v1.92.6"
)
FetchContent_MakeAvailable(imgui)
add_library(imgui INTERFACE)
target_sources(imgui INTERFACE
	"${imgui_SOURCE_DIR}/imgui.cpp"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/imgui_tables.cpp"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)


target_link_libraries(Demo Octree glfw glm::glm imgui glad_gl)

