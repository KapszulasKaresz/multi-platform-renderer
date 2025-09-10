#ifndef RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/render_target/inc/render_target_window.hpp"

namespace renderer {
namespace render_target {

class RenderTargetWindowVulkan : public RenderTargetWindow {
public:
    RenderTargetWindowVulkan&
        setSurface(VkSurfaceKHR f_surface, const vk::raii::Instance& f_instance);

    vk::raii::SurfaceKHR& getSurface();
private:
    vk::raii::SurfaceKHR m_surface{ nullptr };
};

}   // namespace render_target
}   // namespace renderer

#endif
