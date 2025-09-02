set(STANDALONE_PROJECT_NAME "${PROJECT_NAME}_standalone")

add_executable(${STANDALONE_PROJECT_NAME} src/main.cpp)

target_link_libraries(${STANDALONE_PROJECT_NAME} PRIVATE renderer::renderer)