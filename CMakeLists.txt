cmake_minimum_required(VERSION 3.12)

project(PurrfectEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(deps/glfw)

get_target_property(GLFW_INCLUDE_DIRS glfw INTERFACE_INCLUDE_DIRECTORIES)

add_subdirectory(core)
add_subdirectory(engine)