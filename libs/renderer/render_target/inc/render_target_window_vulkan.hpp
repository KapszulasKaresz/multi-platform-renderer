#ifndef RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/render_target/inc/render_target_window.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace render_target {

class RenderTargetWindowVulkan : public RenderTargetWindow {
public:
    RenderTargetWindowVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    RenderTargetWindowVulkan& setWindow(window::Window* f_window) override final;

    RenderTargetWindowVulkan&
        setSurface(VkSurfaceKHR f_surface, const vk::raii::Instance& f_instance);
    
    RenderTargetWindowVulkan& create();

    vk::raii::SurfaceKHR& getSurface();

private:
    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::SurfaceKHR m_surface{ nullptr };
};

}   // namespace render_target
}   // namespace renderer

#endif
