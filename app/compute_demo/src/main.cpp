#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input_parser/inc/input_parser.hpp"
#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"
#include "renderer/rendering_api/inc/rendering_api_dx.hpp"
#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"
#include "renderer/uniform/inc/uniform_single.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer.hpp"
#include "renderer/window/inc/window.hpp"

int main(int argc, const char* argv[])
{
    InputParser l_inputs(argc, argv);
    auto& l_renderingServer = renderer::rendering_server::RenderingServer::getInstance();
    l_renderingServer.setRenderMode(renderer::rendering_server::RENDER_MODE_LOOP);

    if (l_inputs.getCmdOption("--api").value_or("") == "dx") {
        auto l_renderingApi = std::make_unique<renderer::rendering_api::RenderingApiDX>();

        l_renderingApi->enableDebuging().setUseImGui(false).create();

        l_renderingServer.setRenderingApi(std::move(l_renderingApi)).create();
    }
    else {
        auto l_renderingApi =
            std::make_unique<renderer::rendering_api::RenderingApiVulkan>();

        l_renderingApi->enableValidationLayers(true)
            .addValidationLayer("VK_LAYER_KHRONOS_validation")
            .addExtension(vk::EXTDebugUtilsExtensionName)
            .setUseImGui(false)
            .create();

        l_renderingServer.setRenderingApi(std::move(l_renderingApi)).create();
    }

    auto l_computeDevice   = l_renderingServer.getMainRenderingDevice();
    auto l_computeMaterial = l_computeDevice->createMaterial();
    l_computeMaterial
        ->setMaterialType(renderer::material::MaterialType::MATERIAL_TYPE_COMPUTE)
        .setShader("res/shaders/compiled/compute");

    auto l_computeBuffer = l_computeDevice->createUniformStorageBuffer();

    std::array<glm::vec3, 4> l_computeData{
        glm::vec3(1.0f), glm::vec3(2.0f), glm::vec3(3.0f), glm::vec3(4.0f)
    };

    l_computeBuffer->setName("ComputeBuffer").create();
    l_computeBuffer->setValue(l_computeData);

    l_computeMaterial->addUniformStorageBuffer(l_computeBuffer);

    l_computeMaterial->create();

    auto l_commandBuffer = l_computeDevice->createCommandBuffer();
    l_commandBuffer->reset();
    l_commandBuffer->begin();
    l_commandBuffer->useMaterial(l_computeMaterial);
    l_commandBuffer->dispatchCompute(4, 1, 1);
    l_commandBuffer->syncStorageBuffer(l_computeBuffer.get());
    l_commandBuffer->dispatchCompute(4, 1, 1);
    l_commandBuffer->end();
    l_commandBuffer->submit();

    l_computeData.fill(glm::vec3(0.0f));

    l_computeBuffer->getValue(l_computeData);

    std::cout << "API used: "
              << (l_renderingServer.getRenderingApi()->getRenderingAPIType()
                          == renderer::rendering_api::RENDERING_API_TYPE_VULKAN
                      ? "Vulkan"
                      : "DirectX 12")
              << std::endl;
    std::cout << "Device used: " << l_computeDevice->getDeviceName() << std::endl;

    std::cout << "Compute data:" << std::endl;
    for (const auto& value : l_computeData) {
        std::cout << " " << value.x << ", " << value.y << ", " << value.z << ";"
                  << std::endl;
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
