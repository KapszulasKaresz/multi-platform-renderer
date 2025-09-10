#include "renderer/render_target/inc/render_target_window_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace render_target {
RenderTargetWindowVulkan::RenderTargetWindowVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : RenderTargetWindow(),
      m_parentDevice(f_parentDevice)
{}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::setSurface(
    VkSurfaceKHR              f_surface,
    const vk::raii::Instance& f_instance
)
{
    m_surface = vk::raii::SurfaceKHR(f_instance, f_surface);
    return *this;
}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::create()
{
    m_valid = true;
    return *this;
}

vk::raii::SurfaceKHR& RenderTargetWindowVulkan::getSurface()
{
    return m_surface;
}

}   // namespace render_target
}   // namespace renderer
