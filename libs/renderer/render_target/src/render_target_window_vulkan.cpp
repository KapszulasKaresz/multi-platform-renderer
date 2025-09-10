#include "renderer/render_target/inc/render_target_window_vulkan.hpp"

namespace renderer {
namespace render_target {
RenderTargetWindowVulkan& RenderTargetWindowVulkan::setSurface(VkSurfaceKHR f_surface, const vk::raii::Instance& f_instance)
{
    m_surface = vk::raii::SurfaceKHR(f_instance, f_surface);
    return *this;
}

vk::raii::SurfaceKHR& RenderTargetWindowVulkan::getSurface()
{
    return m_surface;
}

}   // namespace render_target
}   // namespace renderer
