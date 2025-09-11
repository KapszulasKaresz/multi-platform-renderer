set(RENDERER_LIB_NAME "${PROJECT_NAME}_renderer")

add_library(${RENDERER_LIB_NAME})
add_library(renderer::renderer ALIAS ${RENDERER_LIB_NAME})

target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE CMAKE_EXPORT_COMPILE_COMMANDS=1)
target_include_directories(${RENDERER_LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libs)

include(cmake/dependencies.cmake)

add_subdirectory(image)
add_subdirectory(render_target)
add_subdirectory(rendering_api)
add_subdirectory(rendering_device)
add_subdirectory(rendering_server)
add_subdirectory(window)
