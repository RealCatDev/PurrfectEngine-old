file(GLOB_RECURSE CORE_SOURCES "src/**.cpp")
add_library(core STATIC ${CORE_SOURCES} ${IMGUI_FILES})
target_include_directories(core PUBLIC "./include/" ${DEFAULT_INCLUDES})
target_link_libraries(core ${DEFAULT_LIBS})