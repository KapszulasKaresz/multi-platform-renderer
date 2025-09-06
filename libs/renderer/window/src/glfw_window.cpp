#include "renderer/window/inc/glfw_window.hpp"

#include <stdexcept>

namespace renderer {
namespace window {
int GLFWWindow::s_numberOfWindows{ 0 };

std::shared_ptr<render_target::RenderTarget> GLFWWindow::getRenderTarget(
    rendering_api::RenderingApi* f_renderingApi
) const
{
    std::shared_ptr<render_target::RenderTarget> l_renderTarget;
    switch (f_renderingApi->getRenderingAPIType()) {
        case rendering_api::RENDERING_API_TYPE_VULKAN: {
            break;
        }
        default: {
            throw std::runtime_error(
                "GLFWWindow::getRenderTarget(rendering_api::RenderingApi* "
                "f_renderingApi) no implementation for this type of API"
            );
            break;
        }
    }

    return l_renderTarget;
}

Window& GLFWWindow::create()
{
    if (s_numberOfWindows == 0) {
        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("GLFWWindow::create() failed to init glfw");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    m_window = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);

    s_numberOfWindows++;
    m_created = true;
    return *this;
}

bool GLFWWindow::isOpen()
{
    return !glfwWindowShouldClose(m_window);
}

void GLFWWindow::update()
{
    glfwPollEvents();
}

std::vector<const char*> GLFWWindow::getRequiredInstanceExtensionsVulkan()
{
    uint32_t l_extensionCount = 0;
    auto     l_extension      = glfwGetRequiredInstanceExtensions(&l_extensionCount);

    return std::vector(l_extension, l_extension + l_extensionCount);
}

GLFWWindow::~GLFWWindow()
{
    glfwDestroyWindow(m_window);
    s_numberOfWindows--;
    if (s_numberOfWindows == 0) {
        glfwTerminate();
    }
}
}   // namespace window
}   // namespace renderer
