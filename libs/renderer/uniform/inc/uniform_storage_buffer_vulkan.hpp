#ifndef UNIFORM_STORAGE_BUFFER_VULKAN_HPP_INCLUDED
#define UNIFORM_STORAGE_BUFFER_VULKAN_HPP_INCLUDED

#include "renderer/uniform/inc/uniform_storage_buffer.hpp"
#include "renderer/uniform/inc/uniform_vulkan.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace uniform {
class UniformStorageBufferVulkan : public UniformStorageBuffer, public UniformVulkan {
public:
    UniformStorageBufferVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    UniformStorageBufferVulkan& create() override final;

    void setValueRaw(std::span<const std::byte> f_dataView) override;
    void getValueRaw(std::span<std::byte> f_outBuffer) override;

    vk::DescriptorSetLayout  getDescriptorSetLayout() const override final;
    vk::raii::DescriptorSet& getDescriptorSet() override final;

    size_t getAlignment() const override final;

protected:
    void createDescriptorSetLayout();
    void createDescriptorSet();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr };

    vk::Flags<vk::ShaderStageFlagBits> m_shaderStage{
        vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eAllGraphics
    };

    std::vector<vk::raii::DescriptorSet> m_descriptorSets{};

    utils::VmaBuffer m_buffer;
};

}   // namespace uniform
}   // namespace renderer

#endif
