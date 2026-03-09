#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace render_target {
class RenderTargetWindow;
}   // namespace render_target

namespace rendering_api {
class RenderingApi;
}   // namespace rendering_api

namespace window {

enum Keys {
    KEY_UNKNOWN,
    KEY_SPACE,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_EQUAL,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_MAX
};

enum MouseButtons {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_MAX
};

class Window : public RenderResource {
public:
    Window() = default;

    Window& setSize(const glm::ivec2& f_size);
    Window& setTitle(const std::string& f_title);

    virtual VkSurfaceKHR createVulkanSurface(const vk::raii::Instance& f_instance) = 0;

    virtual Window& create()        = 0;
    virtual bool    isOpen()        = 0;
    virtual void    update()        = 0;
    virtual void    waitTillShown() = 0;
    virtual void*   getHwnd()       = 0;

    virtual glm::ivec2                getCursorPosition() const      = 0;
    virtual std::vector<MouseButtons> getPressedMouseButtons() const = 0;
    virtual std::vector<Keys>         getPressedKeys() const         = 0;

    virtual int  getWidth() const;
    virtual int  getHeight() const;
    virtual bool isResized() const;
    virtual void resizeHandled(bool f_resized = false);

    virtual ~Window() = default;

protected:
    glm::ivec2  m_size{ 600, 600 };
    std::string m_title{ "empty window" };
    bool        m_framebufferResized{ false };
};
}   // namespace window
}   // namespace renderer

#endif
