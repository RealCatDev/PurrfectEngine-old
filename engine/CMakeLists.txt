file(GLOB_RECURSE ENGINE_SOURCES "src/**.cpp")
add_executable(engine ${ENGINE_SOURCES} ${IMGUI_FILES})
target_include_directories(engine PUBLIC "./include/" "../core/include/" ${DEFAULT_INCLUDES})
target_link_libraries(engine core ${DEFAULT_LIBS})