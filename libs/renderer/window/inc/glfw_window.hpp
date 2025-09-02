#ifndef GLFW_WINDOW_HPP_INCLUDED
#define GLFW_WINDOW_HPP_INCLUDED

#include "renderer/window/inc/window.hpp"

#include <GLFW/glfw3.h>
#include <memory>

namespace renderer {
namespace window {
class GLFWWindow : public Window{
public:
    GLFWWindow();

    ~GLFWWindow() override final;
private:
    std::unique_ptr<GLFWwindow> m_window{nullptr};

};
}   // namespace window
}   // namespace renderer

#endif
