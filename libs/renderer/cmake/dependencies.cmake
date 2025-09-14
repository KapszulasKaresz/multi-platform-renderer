# glm
find_package(glm CONFIG REQUIRED)
target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RADIANS
)
target_link_libraries(${RENDERER_LIB_NAME} PUBLIC glm::glm)

#glfw
find_package(glfw3 CONFIG REQUIRED)
target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE
        GLFW_INCLUDE_VULKAN)
target_link_libraries(${RENDERER_LIB_NAME} PUBLIC glfw)

# VulkanMemoryAllocator
find_package(VulkanMemoryAllocator REQUIRED)
target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE)
target_link_libraries(${RENDERER_LIB_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)

#vulkan
find_package(Vulkan REQUIRED)
target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)
target_link_libraries(${RENDERER_LIB_NAME} PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(${RENDERER_LIB_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
