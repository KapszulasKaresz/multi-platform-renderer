#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/uniform/inc/uniform_single_vulkan.hpp"

namespace renderer {
namespace uniform {
UniformCollectionVulkan::UniformCollectionVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : UniformCollection(),
      m_parentDevice(f_parentDevice)
{}

size_t UniformCollectionVulkan::getAlignment() const
{
    return 16;   // Default memory alignment for structs in vulkan
}

Uniform* UniformCollectionVulkan::addMember(std::string_view f_name)
{
    m_members.push_back(std::make_unique<UniformSingleVulkan>());
    return m_members.back().get();
}

UniformCollectionVulkan& UniformCollectionVulkan::setShaderstage(
    vk::ShaderStageFlagBits f_shaderStage
)
{
    m_shaderStage = f_shaderStage;
    return *this;
}

UniformCollectionVulkan& UniformCollectionVulkan::setBinding(uint32_t f_binding)
{
    m_binding = f_binding;
    return *this;
}

UniformCollectionVulkan& UniformCollectionVulkan::create()
{
    Uniform::create();
    createDescriptorSetLayout();

    return *this;
}

vk::DescriptorSetLayout UniformCollectionVulkan::getDescriptorSetLayout()
{
    return *m_descriptorSetLayout;
}

void UniformCollectionVulkan::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding l_layoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1, m_shaderStage, nullptr
    );
    vk::DescriptorSetLayoutCreateInfo l_layoutInfo{ .bindingCount = 1,
                                                    .pBindings    = &l_layoutBinding };
    m_descriptorSetLayout =
        vk::raii::DescriptorSetLayout(m_parentDevice->getLogicalDevice(), l_layoutInfo);
}

}   // namespace uniform
}   // namespace renderer
