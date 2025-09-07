#ifndef RENDERING_DEVICE_VULKAN_HPP_INCLUDED
#define RENDERING_DEVICE_VULKAN_HPP_INCLUDED

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_api {
class RenderingApiVulkan;
}   // namespace rendering_api

namespace rendering_device {
class RenderingDeviceVulkan : public RenderingDevice {
public:
    RenderingDeviceVulkan(rendering_api::RenderingApiVulkan* f_parentApi);

    std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(
        window::Window* f_window
    ) override final;

private:
    rendering_api::RenderingApiVulkan* m_parentApi{ nullptr };
};

}   // namespace rendering_device
}   // namespace renderer

#endif
