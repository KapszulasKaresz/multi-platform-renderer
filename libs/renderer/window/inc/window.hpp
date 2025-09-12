#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include <memory>
#include <string>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include <glm/vec2.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace render_target {
class RenderTargetWindow;
}   // namespace render_target

namespace rendering_api {
class RenderingApi;
}   // namespace rendering_api

namespace window {
class Window : public RenderResource {
public:
    Window() = default;

    Window& setSize(const glm::ivec2& f_size);
    Window& setTitle(const std::string& f_title);

    virtual VkSurfaceKHR createVulkanSurface(const vk::raii::Instance& f_instance) = 0;

    virtual Window& create() = 0;
    virtual bool    isOpen() = 0;
    virtual void    update() = 0;

    virtual int getWidth() const;
    virtual int getHeight() const;

    virtual ~Window() = default;

protected:
    glm::ivec2  m_size{ 600, 600 };
    std::string m_title{ "empty window" };
};
}   // namespace window
}   // namespace renderer

#endif
