#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_vulkan.hpp"

namespace renderer {
namespace ray_tracing {
AccelerationStructureVulkan::AccelerationStructureVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

AccelerationStructureVulkan& AccelerationStructureVulkan::create()
{
    m_valid = true;
    return *this;
}

}   // namespace ray_tracing
}   // namespace renderer
