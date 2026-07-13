#include "renderer/uniform/inc/uniform_storage_buffer_vulkan.hpp"

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace uniform {
UniformStorageBufferVulkan::UniformStorageBufferVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : UniformStorageBuffer(),
      UniformVulkan(),
      m_parentDevice(f_parentDevice)
{}

UniformStorageBufferVulkan& UniformStorageBufferVulkan::create()
{
    m_valid = true;
    createDescriptorSetLayout();
    return *this;
}

void UniformStorageBufferVulkan::setValueRaw(std::span<const std::byte> f_dataView)
{
    m_size   = f_dataView.size();
    m_buffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        f_dataView.size(),
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
            | vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    auto l_stagingBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        f_dataView.size(),
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        0
    );

    l_stagingBuffer.upload(f_dataView.data(), f_dataView.size());

    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->copyBuffer(l_stagingBuffer, m_buffer, f_dataView.size());
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();

    createDescriptorSet();
}

void UniformStorageBufferVulkan::getValueRaw(std::span<std::byte> f_outBuffer)
{
    auto l_stagingBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        f_outBuffer.size(),
        vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_TO_CPU,
        0
    );

    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->copyBuffer(m_buffer, l_stagingBuffer, f_outBuffer.size());
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();

    std::memcpy(
        f_outBuffer.data(), l_stagingBuffer.copyDataToCPU().data(), f_outBuffer.size()
    );
}

vk::DescriptorSetLayout UniformStorageBufferVulkan::getDescriptorSetLayout() const
{
    return m_descriptorSetLayout;
}

vk::raii::DescriptorSet& UniformStorageBufferVulkan::getDescriptorSet()
{
    return m_descriptorSets[m_parentDevice->getCurrentFrame()];
}

size_t UniformStorageBufferVulkan::getAlignment() const
{
    return 16;
}

utils::VmaBuffer& UniformStorageBufferVulkan::getBuffer()
{
    return m_buffer;
}

void UniformStorageBufferVulkan::createDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> l_layoutBindings;

    l_layoutBindings.push_back(
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eStorageBuffer, 1, m_shaderStage, nullptr
        )
    );

    vk::DescriptorSetLayoutCreateInfo l_layoutInfo{
        .bindingCount = static_cast<uint32_t>(l_layoutBindings.size()),
        .pBindings    = l_layoutBindings.data()
    };
    m_descriptorSetLayout =
        vk::raii::DescriptorSetLayout(m_parentDevice->getLogicalDevice(), l_layoutInfo);
}

void UniformStorageBufferVulkan::createDescriptorSet()
{
    if (m_descriptorSets.empty()) {
        std::vector<vk::DescriptorSetLayout> l_layouts(
            m_parentDevice->getMaxFramesInFlight(), *m_descriptorSetLayout
        );
        vk::DescriptorSetAllocateInfo l_allocInfo{
            .descriptorPool     = m_parentDevice->getDescriptorPool(),
            .descriptorSetCount = static_cast<uint32_t>(l_layouts.size()),
            .pSetLayouts        = l_layouts.data()
        };

        m_descriptorSets =
            m_parentDevice->getLogicalDevice().allocateDescriptorSets(l_allocInfo);
    }

    for (size_t i = 0; i < m_parentDevice->getMaxFramesInFlight(); i++) {
        std::vector<vk::WriteDescriptorSet> l_descriptorWrites;
        vk::DescriptorBufferInfo            l_bufferInfo{ .buffer = m_buffer.get(),
                                                          .offset = 0,
                                                          .range  = m_size };

        l_descriptorWrites.push_back(
            vk::WriteDescriptorSet{ .dstSet          = m_descriptorSets[i],
                                    .dstBinding      = 0,
                                    .dstArrayElement = 0,
                                    .descriptorCount = 1,
                                    .descriptorType  = vk::DescriptorType::eStorageBuffer,
                                    .pBufferInfo     = &l_bufferInfo }
        );
        m_parentDevice->getLogicalDevice().updateDescriptorSets(l_descriptorWrites, {});
    }
}


}   // namespace uniform
}   // namespace renderer
