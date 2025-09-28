#ifndef UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED
#define UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED

#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/uniform/inc/uniform_collection.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace uniform {
class UniformCollectionVulkan : public UniformCollection {
public:
    UniformCollectionVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    void   update() override final;
    size_t getSize() const override final;

    size_t         getAlignment() const override final;
    UniformSingle* addMember(const std::string& f_name) override final;

    UniformCollectionVulkan& setShaderstage(vk::ShaderStageFlagBits f_shaderStage);
    UniformCollectionVulkan& setSet(uint32_t f_set);
    uint32_t                 getSet() const;

    UniformCollectionVulkan& create() override final;

    vk::DescriptorSetLayout  getDescriptorSetLayout() const;
    vk::raii::DescriptorSet& getDescriptorSet();

private:
    void createDescriptorSetLayout();
    void createUniformBuffers();
    void createDescriptorSets();

    void                 computeStd140Layout();
    std::vector<uint8_t> createStd140Buffer();

    struct Layout {
        size_t              m_structSize;
        std::vector<size_t> m_offsets;
    } m_layout;

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::ShaderStageFlagBits       m_shaderStage{ vk::ShaderStageFlagBits::eAllGraphics };
    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr };

    std::vector<vk::raii::DescriptorSet> m_descriptorSets{};

    vk::DeviceSize                m_bufferSize{ 0 };
    std::vector<utils::VmaBuffer> m_uniformBuffers;
};

}   // namespace uniform
}   // namespace renderer

#endif
