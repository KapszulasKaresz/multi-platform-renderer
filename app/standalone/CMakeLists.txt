set(STANDALONE_PROJECT_NAME "${PROJECT_NAME}_standalone")
set(SHADER_COMPILE_STEP "${PROJECT_NAME}_shader_compile")

include(cmake/shaders.cmake)

set(SHADER_SLANG_SOURCES
    ${CMAKE_SOURCE_DIR}/res/shaders/shader.slang
)

add_slang_shader_target(${SHADER_COMPILE_STEP} SOURCES ${SHADER_SLANG_SOURCES})

add_executable(${STANDALONE_PROJECT_NAME}
    src/main.cpp
    src/input_parser.cpp)

target_include_directories(${STANDALONE_PROJECT_NAME} PUBLIC inc)

target_link_libraries(${STANDALONE_PROJECT_NAME} PRIVATE renderer::renderer)

add_dependencies(${STANDALONE_PROJECT_NAME} ${SHADER_COMPILE_STEP})
