#include "renderer/window/inc/glfw_window.hpp"

#include <stdexcept>

namespace renderer {
namespace window {
    int GLFWWindow::s_numberOfWindows{0};

    std::shared_ptr<render_target::RenderTarget> GLFWWindow::getRenderTarget() const
    {
        return std::make_unique<render_target::RenderTarget>();
    }


    Window& GLFWWindow::create() 
    {
        if(s_numberOfWindows == 0)
        {
            if(glfwInit() != GLFW_TRUE)
            {
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

    GLFWWindow::~GLFWWindow()
    {
        glfwDestroyWindow(m_window);
        s_numberOfWindows--;
        if(s_numberOfWindows == 0)
        {
            glfwTerminate();
        }
    }
}
}   // namespace renderer
