#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
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

        auto l_glfwExtension = renderer::window::GLFWWindow::getRequiredInstanceExtensionsVulkan();

        l_renderingApi->enableValidationLayers(true)
            .addValidationLayer("VK_LAYER_KHRONOS_validation")
            .addExtensions(l_glfwExtension)
            .create();

        l_renderingServer.setRenderingApi(std::move(l_renderingApi));
        l_renderingServer.setWindow(std::move(l_window));

        l_renderingServer.mainLoop();
    } catch (const std::runtime_error& l_error) {
        std::cout << "Something went wrong: " << l_error.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
