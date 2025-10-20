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
        GLFW_INCLUDE_VULKAN
        GLFW_EXPOSE_NATIVE_WIN32)
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

#stb
find_package(stb REQUIRED)
target_compile_definitions(${RENDERER_LIB_NAME} PRIVATE
        STB_IMAGE_IMPLEMENTATION)
target_link_libraries(${RENDERER_LIB_NAME} PRIVATE stb::stb)

#directX
if(WIN32)
        include_directories("$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}/um")
        include_directories("$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}/shared")
        include_directories("$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}/winrt")
        include_directories("$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}/cppwinrt")

        link_directories("$ENV{WindowsSdkDir}/Lib/$ENV{WindowsSDKVersion}/um/x64")

        target_link_libraries(${RENDERER_LIB_NAME} PUBLIC
                d3d12
                dxgi
                dxguid
                d3dcompiler
        )
endif()
