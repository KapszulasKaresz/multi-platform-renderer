#ifndef GLFW_WINDOW_HPP_INCLUDED
#define GLFW_WINDOW_HPP_INCLUDED

#include <memory>
#include <vector>

#include <GLFW/glfw3.h>

#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace window {
class GLFWWindow : public Window {
public:
    GLFWWindow() = default;

    VkSurfaceKHR createVulkanSurface(const vk::raii::Instance& f_instance) override final;
    Window&      create() override final;
    bool         isOpen() override final;
    void         update() override final;
    void         waitTillShown() override final;
    void*        getHwnd() override final;

    static std::vector<const char*> getRequiredInstanceExtensionsVulkan();

    ~GLFWWindow() override final;

private:
    static void framebufferResizeCallback(GLFWwindow* f_window, int f_width, int f_height);

    GLFWwindow* m_window{ nullptr };

    static int s_numberOfWindows;
};
}   // namespace window
}   // namespace renderer

#endif
