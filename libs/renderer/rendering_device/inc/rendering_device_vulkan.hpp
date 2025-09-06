#ifndef RENDERING_DEVICE_VULKAN_HPP_INCLUDED
#define RENDERING_DEVICE_VULKAN_HPP_INCLUDED

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan : public RenderingDevice {
public:
    RenderingDeviceVulkan();

    std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(
        const window::Window* f_window
    ) override final;

private:
};

}   // namespace rendering_device
}   // namespace renderer

#endif
