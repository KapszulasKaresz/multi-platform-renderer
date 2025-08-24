find_package(glfw3 3.4 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    glfw    
)