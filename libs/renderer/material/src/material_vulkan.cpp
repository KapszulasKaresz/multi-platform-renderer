#include "renderer/material/inc/material_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace material {
MaterialVulkan::MaterialVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice)
    : Material(),
      m_parentDevice(f_parentDevice)
{}

MaterialVulkan& MaterialVulkan::create()
{
    m_valid = true;
    return *this;
}

}   // namespace material
}   // namespace renderer
