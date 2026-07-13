set(COMPUTE_DEMO_PROJECT_NAME "${PROJECT_NAME}_compute_demo")

add_executable(${COMPUTE_DEMO_PROJECT_NAME}
    src/main.cpp)

target_link_libraries(${COMPUTE_DEMO_PROJECT_NAME} PRIVATE renderer::renderer)
target_link_libraries(${COMPUTE_DEMO_PROJECT_NAME} PRIVATE app::common)

