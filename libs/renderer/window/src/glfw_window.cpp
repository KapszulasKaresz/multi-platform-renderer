#include "renderer/window/inc/glfw_window.hpp"

#include <stdexcept>

#include <GLFW/glfw3native.h>

namespace renderer {
namespace window {
int GLFWWindow::s_numberOfWindows{ 0 };

VkSurfaceKHR GLFWWindow::createVulkanSurface(const vk::raii::Instance& f_instance)
{
    VkSurfaceKHR l_surface;
    if (glfwCreateWindowSurface(*f_instance, m_window, nullptr, &l_surface) != 0) {
        throw std::runtime_error(
            "GLFWWindow::createVulkanSurface(const vk::raii::Instance& f_instance) "
            "failed to create window surface!"
        );
    }
    return l_surface;
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
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    s_numberOfWindows++;
    m_valid = true;
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

void GLFWWindow::waitTillShown()
{
    int l_width = 0, l_height = 0;
    glfwGetFramebufferSize(m_window, &l_width, &l_height);
    while (l_width == 0 || l_height == 0) {
        glfwGetFramebufferSize(m_window, &l_width, &l_height);
        glfwWaitEvents();
    }
}

void* GLFWWindow::getHwnd()
{
    return glfwGetWin32Window(m_window);
}

glm::ivec2 GLFWWindow::getCursorPosition() const
{
    double l_xPos, l_yPos;
    glfwGetCursorPos(m_window, &l_xPos, &l_yPos);
    return glm::ivec2(static_cast<int>(l_xPos), static_cast<int>(l_yPos));
}

std::vector<MouseButtons> GLFWWindow::getPressedMouseButtons() const
{
    std::vector<MouseButtons> l_pressedButtons;
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        l_pressedButtons.push_back(MouseButtons::MOUSE_BUTTON_LEFT);
    }
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        l_pressedButtons.push_back(MouseButtons::MOUSE_BUTTON_RIGHT);
    }
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        l_pressedButtons.push_back(MouseButtons::MOUSE_BUTTON_MIDDLE);
    }
    return l_pressedButtons;
}

std::vector<Keys> GLFWWindow::getPressedKeys() const
{
    std::vector<Keys> l_pressedKeys;

    static const int s_keysToPoll[] = {
        GLFW_KEY_SPACE,     GLFW_KEY_APOSTROPHE, GLFW_KEY_COMMA, GLFW_KEY_MINUS,
        GLFW_KEY_PERIOD,    GLFW_KEY_SLASH,      GLFW_KEY_0,     GLFW_KEY_1,
        GLFW_KEY_2,         GLFW_KEY_3,          GLFW_KEY_4,     GLFW_KEY_5,
        GLFW_KEY_6,         GLFW_KEY_7,          GLFW_KEY_8,     GLFW_KEY_9,
        GLFW_KEY_SEMICOLON, GLFW_KEY_EQUAL,      GLFW_KEY_A,     GLFW_KEY_B,
        GLFW_KEY_C,         GLFW_KEY_D,          GLFW_KEY_E,     GLFW_KEY_F,
        GLFW_KEY_G,         GLFW_KEY_H,          GLFW_KEY_I,     GLFW_KEY_J,
        GLFW_KEY_K,         GLFW_KEY_L,          GLFW_KEY_M,     GLFW_KEY_N,
        GLFW_KEY_O,         GLFW_KEY_P,          GLFW_KEY_Q,     GLFW_KEY_R,
        GLFW_KEY_S,         GLFW_KEY_T,          GLFW_KEY_U,     GLFW_KEY_V,
        GLFW_KEY_W,         GLFW_KEY_X,          GLFW_KEY_Y,     GLFW_KEY_Z
    };

    static Keys s_keyMap[GLFW_KEY_LAST + 1] = { KEY_UNKNOWN };
    static bool s_initialized               = false;

    if (!s_initialized) {
        for (int i = 0; i <= 9; ++i) {
            s_keyMap[GLFW_KEY_0 + i] = static_cast<Keys>(KEY_0 + i);
        }

        for (int i = 0; i <= 25; ++i) {
            s_keyMap[GLFW_KEY_A + i] = static_cast<Keys>(KEY_A + i);
        }

        s_keyMap[GLFW_KEY_SPACE]      = KEY_SPACE;
        s_keyMap[GLFW_KEY_APOSTROPHE] = KEY_APOSTROPHE;
        s_keyMap[GLFW_KEY_COMMA]      = KEY_COMMA;
        s_keyMap[GLFW_KEY_MINUS]      = KEY_MINUS;
        s_keyMap[GLFW_KEY_PERIOD]     = KEY_PERIOD;
        s_keyMap[GLFW_KEY_SLASH]      = KEY_SLASH;
        s_keyMap[GLFW_KEY_SEMICOLON]  = KEY_SEMICOLON;
        s_keyMap[GLFW_KEY_EQUAL]      = KEY_EQUAL;

        s_initialized = true;
    }

    for (int l_glfwKey : s_keysToPoll) {
        if (glfwGetKey(m_window, l_glfwKey) == GLFW_PRESS) {
            l_pressedKeys.push_back(s_keyMap[l_glfwKey]);
        }
    }

    return l_pressedKeys;
}

std::vector<const char*> GLFWWindow::getRequiredInstanceExtensionsVulkan()
{
    uint32_t l_extensionCount = 0;
    auto     l_extension      = glfwGetRequiredInstanceExtensions(&l_extensionCount);

    return std::vector(l_extension, l_extension + l_extensionCount);
}

GLFWwindow* GLFWWindow::getNativeHandle()
{
    return m_window;
}

GLFWWindow::~GLFWWindow()
{
    glfwDestroyWindow(m_window);
    s_numberOfWindows--;
    if (s_numberOfWindows == 0) {
        glfwTerminate();
    }
}

void GLFWWindow::framebufferResizeCallback(GLFWwindow* f_window, int f_width, int f_height)
{
    auto l_window = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(f_window));
    l_window->resizeHandled(true);
    l_window->setSize(glm::ivec2(f_width, f_height));
}


}   // namespace window
}   // namespace renderer
