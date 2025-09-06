#ifndef GLFW_WINDOW_HPP_INCLUDED
#define GLFW_WINDOW_HPP_INCLUDED

#include "renderer/window/inc/window.hpp"

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace renderer {
namespace window {
class GLFWWindow : public Window{
public:
    GLFWWindow() = default;

    Window& create() override final;
    bool isOpen() override final;
    void update() override final;

    static std::vector<const char*> getRequiredInstanceExtensionsVulkan();

    ~GLFWWindow() override final;
private:
    GLFWwindow* m_window{nullptr};

    static int s_numberOfWindows;
};
}   // namespace window
}   // namespace renderer

#endif
