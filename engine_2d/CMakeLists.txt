file(GLOB_RECURSE ENGINE2D_SOURCES "src/**.cpp")
add_executable(engine2d ${ENGINE2D_SOURCES} ${IMGUI_FILES})
target_include_directories(engine2d PUBLIC "./include/" "../core/include/" ${DEFAULT_INCLUDES})
target_link_libraries(engine2d core ${DEFAULT_LIBS})