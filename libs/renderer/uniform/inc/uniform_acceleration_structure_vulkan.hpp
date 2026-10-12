#ifndef UNIFORM_ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED
#define UNIFORM_ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"
#include "renderer/uniform/inc/uniform_vulkan.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}

namespace uniform {
class UniformAccelerationStructureVulkan final
    : public UniformAccelerationStructure,
      public UniformVulkan {
public:
    UniformAccelerationStructureVulkan(
        rendering_device::RenderingDeviceVulkan* f_parentDevice
    );

    UniformAccelerationStructureVulkan& create() override final;

    vk::DescriptorSetLayout getDescriptorSetLayout() const override final;
    vk::raii::DescriptorSet& getDescriptorSet() override final;

private:
    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };
    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr };
    std::vector<vk::raii::DescriptorSet> m_descriptorSets;
};
}   // namespace uniform
}   // namespace renderer

#endif
