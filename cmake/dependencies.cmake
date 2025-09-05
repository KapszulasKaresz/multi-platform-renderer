# glm
find_package(glm CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RADIANS
)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

#glfw
find_package(glfw3 CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

# VulkanMemoryAllocator
find_package(VulkanMemoryAllocator REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)

#volk
find_package(volk REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} PUBLIC volk::volk)

#vulkan
find_package(Vulkan REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
