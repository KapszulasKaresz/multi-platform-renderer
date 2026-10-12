#ifndef ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED
#define ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace ray_tracing {
class AccelerationStructureVulkan : public AccelerationStructure {
public:
    AccelerationStructureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    AccelerationStructureVulkan& create();

protected:
    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::AccelerationStructureTypeKHR m_accelerationStructureType{
        vk::AccelerationStructureTypeKHR::eGeneric
    };

    vk::raii::AccelerationStructureKHR         m_accelerationStructure{ nullptr };
    vk::AccelerationStructureBuildSizesInfoKHR m_accelerationStructureBuildSizesInfo{};

    utils::VmaBuffer m_accelerationStructureBuffer{};
    utils::VmaBuffer m_scratchBuffer{};
};
}   // namespace ray_tracing
}   // namespace renderer

#endif
