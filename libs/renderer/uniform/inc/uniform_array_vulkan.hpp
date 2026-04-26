#ifndef UNIFORM_ARRAY_VULKAN_HPP_INCLUDED
#define UNIFORM_ARRAY_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/uniform/inc/uniform_array.hpp"
#include "renderer/uniform/inc/uniform_vulkan.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace uniform {
class UniformCollection;

class UniformArrayVulkan : public UniformArray, public UniformVulkan {
public:
    UniformArrayVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    UniformArrayVulkan& setShaderstage(vk::ShaderStageFlagBits f_shaderStage);

    UniformArrayVulkan& create() override final;

    size_t getAlignment() const override final;

    void update() override final;

    vk::DescriptorSetLayout  getDescriptorSetLayout() const override final;
    vk::raii::DescriptorSet& getDescriptorSet() override final;

protected:
    void createDescriptorSetLayout();
    void createDescriptorSets();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::ShaderStageFlagBits       m_shaderStage{ vk::ShaderStageFlagBits::eAllGraphics };
    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr };

    std::vector<vk::raii::DescriptorSet> m_descriptorSets{};

    std::shared_ptr<UniformCollection> m_elementCountCollection{ nullptr };
};

}   // namespace uniform
}   // namespace renderer


#endif
