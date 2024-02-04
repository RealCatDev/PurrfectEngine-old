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

get_target_property(GLFW_INCLUDE_DIRS glfw INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(SPDLOG_INCLUDE_DIRS spdlog INTERFACE_INCLUDE_DIRECTORIES)
set(GLM_DIR deps/glm/)

set(DEFAULT_INCLUDES "../deps/stb/" ${SPDLOG_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_DIR} ${Vulkan_INCLUDE_DIRS})
set(DEFAULT_LIBS spdlog glfw glm Vulkan::Vulkan)

add_subdirectory(core)
add_subdirectory(engine)