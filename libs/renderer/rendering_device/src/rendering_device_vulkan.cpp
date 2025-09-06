#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/render_target/inc/render_target_window.hpp"


namespace renderer {
namespace rendering_device {
    RenderingDeviceVulkan::RenderingDeviceVulkan() : RenderingDevice()
    {

    }

    std::shared_ptr<render_target::RenderTargetWindow> RenderingDeviceVulkan::createRenderTargetWindow(
        const window::Window* f_window
    )
    {
        return std::make_shared<render_target::RenderTargetWindow>();
    }

}   // namespace renderin_device
}   // namespace renderer
