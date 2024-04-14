file(GLOB_RECURSE TEST_SOURCES "src/**.cpp")
add_executable(test ${TEST_SOURCES} ${IMGUI_FILES})
target_include_directories(test PUBLIC "./include/" "../renderer/include/" ${DEFAULT_INCLUDES})
target_link_libraries(test renderer ${DEFAULT_LIBS})