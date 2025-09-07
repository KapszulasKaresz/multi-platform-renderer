#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/render_target/inc/render_target_window_vulkan.hpp"
#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_device {
RenderingDeviceVulkan::RenderingDeviceVulkan(
    rendering_api::RenderingApiVulkan* f_parentApi
)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceVulkan::createRenderTargetWindow(window::Window* f_window)
{
    auto l_renderingWindow = std::make_shared<render_target::RenderTargetWindowVulkan>();
    l_renderingWindow->setSurface(f_window->createVulkanSurface(m_parentApi->getNativeHandle()), m_parentApi->getNativeHandle());
    return l_renderingWindow;
}

}   // namespace rendering_device
}   // namespace renderer
