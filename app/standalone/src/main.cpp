#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/material/inc/material.hpp"
#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/scene/inc/test_scene.hpp"
#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"
#include "renderer/uniform/inc/uniform_single.hpp"
#include "renderer/window/inc/glfw_window.hpp"

int main(int argc, const char* argv[])
{
    try {
        auto& l_renderingServer =
            renderer::rendering_server::RenderingServer::getInstance();
        l_renderingServer.setRenderMode(renderer::rendering_server::RENDER_MODE_LOOP);

        auto l_window = std::make_unique<renderer::window::GLFWWindow>();
        l_window->setSize(glm::ivec2(1'600, 1'200)).setTitle("Test Window").create();

        auto l_renderingApi =
            std::make_unique<renderer::rendering_api::RenderingApiVulkan>();

        auto l_glfwExtension =
            renderer::window::GLFWWindow::getRequiredInstanceExtensionsVulkan();

        l_renderingApi->enableValidationLayers(true)
            .addValidationLayer("VK_LAYER_KHRONOS_validation")
            .addExtensions(l_glfwExtension)
            .addExtension(vk::EXTDebugUtilsExtensionName)
            .create();

        l_renderingServer.setRenderingApi(std::move(l_renderingApi))
            .setWindow(std::move(l_window))
            .create();

        auto l_image = l_renderingServer.getMainRenderingDevice()->createImage();
        l_image->generateMipMaps().createFromFile("res/textures/test_texture.png");

        auto l_texture = l_renderingServer.getMainRenderingDevice()->createTexture();
        l_texture->setImage(l_image).create();

        auto l_uniformCollection =
            l_renderingServer.getMainRenderingDevice()->createUniformCollection();
        l_uniformCollection->addMember("model")
            ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
            .setValue(
                rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f))
            )
            .create();

        l_uniformCollection->addMember("view")
            ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
            .setValue(lookAt(
                glm::vec3(2.0f, 2.0f, 3.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            ))
            .create();

        auto l_proj = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(1'600) / static_cast<float>(1'200),
            0.1f,
            10.0f
        );
        l_proj[1][1] *= -1;
        l_uniformCollection->addMember("proj")
            ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
            .setValue(l_proj)
            .create();

        l_uniformCollection->addTexture(l_texture);
        l_uniformCollection->setName("Camera").create();

        auto l_material = l_renderingServer.getMainRenderingDevice()->createMaterial();
        l_material->setShader("res/shaders/shader.spv")
            .addUniformCollection(l_uniformCollection)
            .create();

        auto l_mesh = l_renderingServer.getMainRenderingDevice()->createTriangleMesh();
        l_mesh->create();

        auto l_scene = std::make_shared<renderer::scene::TestScene>();
        l_scene->setMaterial(l_material).setMesh(l_mesh).create();

        l_renderingServer.setScene(l_scene);

        l_renderingServer.mainLoop();
    } catch (const std::runtime_error& l_error) {
        std::cout << "Something went wrong: " << l_error.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
