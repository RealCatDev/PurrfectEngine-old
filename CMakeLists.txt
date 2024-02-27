cmake_minimum_required(VERSION 3.12)

find_package(Vulkan REQUIRED)

project(PurrfectEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DPURR_DEBUG)
else()
  add_definitions(-DPURR_RELEASE)
endif()

if(WIN32)
  add_definitions(-DPURR_PLAT_WIN)
elseif(UNIX)
  add_definitions(-DPURR_PLAT_LINUX)
endif()

add_subdirectory(deps/glfw)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/glm)
add_subdirectory(deps/imguizmo)

get_target_property(GLFW_INCLUDE_DIRS glfw INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(SPDLOG_INCLUDE_DIRS spdlog INTERFACE_INCLUDE_DIRECTORIES)
set(GLM_DIR deps/glm/)

set(IMGUI_DIR "../deps/imgui/")
set(DEFAULT_INCLUDES "../deps/stb/" ${IMGUI_DIR} ${SPDLOG_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_DIR} ${Vulkan_INCLUDE_DIRS})
set(DEFAULT_LIBS spdlog glfw glm Vulkan::Vulkan ImGuizmo)

set(IMGUI_FILES ${IMGUI_DIR}backends/imgui_impl_glfw.cpp ${IMGUI_DIR}backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}imgui.cpp ${IMGUI_DIR}imgui_draw.cpp ${IMGUI_DIR}imgui_demo.cpp ${IMGUI_DIR}imgui_tables.cpp ${IMGUI_DIR}imgui_widgets.cpp)

add_subdirectory(core)
add_subdirectory(engine)