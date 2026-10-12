#include "renderer/uniform/inc/uniform_acceleration_structure_vulkan.hpp"

#include <stdexcept>

#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace uniform {
UniformAccelerationStructureVulkan::UniformAccelerationStructureVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

UniformAccelerationStructureVulkan&
    UniformAccelerationStructureVulkan::create()
{
    if (!m_accelerationStructure) {
        throw std::runtime_error(
            "UniformAccelerationStructureVulkan::create() requires an acceleration "
            "structure"
        );
    }

    const vk::DescriptorSetLayoutBinding l_binding{
        0,
        vk::DescriptorType::eAccelerationStructureKHR,
        1,
        vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
            | vk::ShaderStageFlagBits::eMissKHR,
        nullptr
    };
    m_descriptorSetLayout = vk::raii::DescriptorSetLayout(
        m_parentDevice->getLogicalDevice(),
        { .bindingCount = 1, .pBindings = &l_binding }
    );

    std::vector<vk::DescriptorSetLayout> l_layouts(
        m_parentDevice->getMaxFramesInFlight(), *m_descriptorSetLayout
    );
    m_descriptorSets = m_parentDevice->getLogicalDevice().allocateDescriptorSets(
        { .descriptorPool     = m_parentDevice->getDescriptorPool(),
          .descriptorSetCount = static_cast<uint32_t>(l_layouts.size()),
          .pSetLayouts        = l_layouts.data() }
    );

    auto* l_vulkanAccelerationStructure =
        dynamic_cast<ray_tracing::AccelerationStructureVulkan*>(
            m_accelerationStructure.get()
        );
    if (!l_vulkanAccelerationStructure) {
        throw std::runtime_error(
            "UniformAccelerationStructureVulkan::create() requires a Vulkan "
            "acceleration structure"
        );
    }

    const vk::AccelerationStructureKHR l_handle =
        *l_vulkanAccelerationStructure->getAccelerationStructure();
    const vk::WriteDescriptorSetAccelerationStructureKHR l_writeAccelerationStructure{
        .accelerationStructureCount = 1,
        .pAccelerationStructures    = &l_handle
    };
    for (auto& l_descriptorSet : m_descriptorSets) {
        m_parentDevice->getLogicalDevice().updateDescriptorSets(
            { vk::WriteDescriptorSet{
                .pNext           = &l_writeAccelerationStructure,
                .dstSet          = l_descriptorSet,
                .dstBinding      = 0,
                .descriptorCount = 1,
                .descriptorType  = vk::DescriptorType::eAccelerationStructureKHR
            } },
            {}
        );
    }
    m_valid = true;
    return *this;
}

vk::DescriptorSetLayout
    UniformAccelerationStructureVulkan::getDescriptorSetLayout() const
{
    return *m_descriptorSetLayout;
}

vk::raii::DescriptorSet& UniformAccelerationStructureVulkan::getDescriptorSet()
{
    return m_descriptorSets[m_parentDevice->getCurrentFrame()];
}
}   // namespace uniform
}   // namespace renderer
