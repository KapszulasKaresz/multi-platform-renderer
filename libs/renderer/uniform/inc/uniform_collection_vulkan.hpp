#ifndef UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED
#define UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace uniform {
class UniformCollectionVulkan : public UniformCollection {
public:
    UniformCollectionVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    size_t   getAlignment() const override final;
    Uniform* addMember(std::string_view f_name) override final;

    UniformCollectionVulkan& setShaderstage(vk::ShaderStageFlagBits f_shaderStage);
    UniformCollectionVulkan& setBinding(uint32_t f_binding);

    UniformCollectionVulkan& create() override final;

    vk::DescriptorSetLayout getDescriptorSetLayout();

private:
    void createDescriptorSetLayout();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    uint32_t                      m_binding{ 0 };
    vk::ShaderStageFlagBits       m_shaderStage{ vk::ShaderStageFlagBits::eVertex };
    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr };
};

}   // namespace uniform
}   // namespace renderer

#endif
