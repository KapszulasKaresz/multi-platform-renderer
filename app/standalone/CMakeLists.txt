find_package(glfw3 3.4 REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    glfw    
)

add_executable(${PROJECT_NAME} src/main.cpp)