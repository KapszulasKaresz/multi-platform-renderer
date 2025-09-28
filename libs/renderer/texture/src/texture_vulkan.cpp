#include "renderer/texture/inc/texture_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace texture {
TextureVulkan::TextureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice)
    : m_parentDevice(f_parentDevice)
{}

TextureVulkan& TextureVulkan::create()
{
    m_valid = true;
    return *this;
}

}   // namespace texture
}   // namespace renderer
