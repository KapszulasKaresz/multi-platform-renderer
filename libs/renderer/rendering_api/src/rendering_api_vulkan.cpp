#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace rendering_api {
std::shared_ptr<rendering_device::RenderingDevice>
    RenderingApiVulkan::createRenderingDevice()
{
    return std::make_shared<rendering_device::RenderingDeviceVulkan>();
}

}   // namespace rendering_api
}   // namespace renderer
