# glm
find_package(glm CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RADIANS
)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)