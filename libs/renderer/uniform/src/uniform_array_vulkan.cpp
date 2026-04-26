#include "renderer/uniform/inc/uniform_array_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"
#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"
#include "renderer/uniform/inc/uniform_single_vulkan.hpp"

namespace renderer {
namespace uniform {
UniformArrayVulkan::UniformArrayVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : UniformArray(),
      m_parentDevice(f_parentDevice)
{}

UniformArrayVulkan& UniformArrayVulkan::setShaderstage(
    vk::ShaderStageFlagBits f_shaderStage
)
{
    m_shaderStage = f_shaderStage;
    return *this;
}

UniformArrayVulkan& UniformArrayVulkan::create()
{
    m_elementCountCollection = m_parentDevice->createUniformCollection();
    m_elementCountCollection->setType(UNIFORM_TYPE_ARRAY_MEMBER)
        .setUnique(false)
        .setName(getName() + "_elementCount");
    m_elementCountCollection->addMember("elementCount")
        ->setType(UNIFORM_TYPE_UINT)
        .setValue(0)
        .create();
    m_elementCountCollection->create();

    if (m_currentElementCount > 0) {
        for (int i = m_currentElementCount; i < m_maxElementCount; ++i) {
            m_elements.push_back(m_elements[0]->deepCopy());
        }
    }

    createDescriptorSetLayout();
    createDescriptorSets();
    m_valid = true;
    return *this;
}

size_t UniformArrayVulkan::getAlignment() const
{
    return 16;   // Default memory alignment for structs in vulkan
}

void UniformArrayVulkan::update()
{
    UniformSingle* l_elementSingleRaw =
        dynamic_cast<UniformSingle*>(m_elementCountCollection->getMember("elementCount"));
    l_elementSingleRaw->setValue(static_cast<uint32_t>(m_currentElementCount));
    m_elementCountCollection->update();

    for (size_t i = 0; i < m_currentElementCount; ++i) {
        if (m_elements[i]) {
            m_elements[i]->update();
        }
    }
}

vk::DescriptorSetLayout UniformArrayVulkan::getDescriptorSetLayout() const
{
    return m_descriptorSetLayout;
}

vk::raii::DescriptorSet& UniformArrayVulkan::getDescriptorSet()
{
    return m_descriptorSets[m_parentDevice->getCurrentFrame()];
}

void UniformArrayVulkan::createDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> l_layoutBindings;


    l_layoutBindings.push_back(
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1, m_shaderStage, nullptr
        )
    );

    if (m_maxElementCount > 0) {
        l_layoutBindings.push_back(
            vk::DescriptorSetLayoutBinding(
                1,
                vk::DescriptorType::eUniformBuffer,
                m_maxElementCount,
                m_shaderStage,
                nullptr
            )
        );
    }


    vk::DescriptorSetLayoutCreateInfo l_layoutInfo{
        .bindingCount = static_cast<uint32_t>(l_layoutBindings.size()),
        .pBindings    = l_layoutBindings.data()
    };
    m_descriptorSetLayout =
        vk::raii::DescriptorSetLayout(m_parentDevice->getLogicalDevice(), l_layoutInfo);
}

void UniformArrayVulkan::createDescriptorSets()
{
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

    for (size_t i = 0; i < m_parentDevice->getMaxFramesInFlight(); i++) {
        std::vector<vk::WriteDescriptorSet> l_descriptorWrites;

        UniformCollectionVulkan* l_elementCountCollectionVulkan =
            dynamic_cast<UniformCollectionVulkan*>(m_elementCountCollection.get());

        vk::DescriptorBufferInfo l_elementCountBufferInfo{
            .buffer = l_elementCountCollectionVulkan->getUniformBuffer(i).get(),
            .offset = 0,
            .range  = l_elementCountCollectionVulkan->getSize()
        };

        l_descriptorWrites.push_back(
            vk::WriteDescriptorSet{ .dstSet          = m_descriptorSets[i],
                                    .dstBinding      = 0,
                                    .dstArrayElement = 0,
                                    .descriptorCount = 1,
                                    .descriptorType  = vk::DescriptorType::eUniformBuffer,
                                    .pBufferInfo     = &l_elementCountBufferInfo }
        );

        std::vector<vk::DescriptorBufferInfo> l_bufferInfos;
        l_bufferInfos.resize(m_maxElementCount);

        for (size_t j = 0; j < m_maxElementCount; ++j) {
            UniformCollectionVulkan* l_collection =
                dynamic_cast<UniformCollectionVulkan*>(m_elements[j].get());

            l_bufferInfos[j] = vk::DescriptorBufferInfo{
                .buffer = l_collection->getUniformBuffer(i).get(),
                .offset = 0,
                .range  = l_collection->getSize()
            };
        }

        l_descriptorWrites.push_back(
            vk::WriteDescriptorSet{
                .dstSet          = m_descriptorSets[i],
                .dstBinding      = 1,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(l_bufferInfos.size()),
                .descriptorType  = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo     = l_bufferInfos.data() }
        );


        m_parentDevice->getLogicalDevice().updateDescriptorSets(l_descriptorWrites, {});
    }
}

}   // namespace uniform
}   // namespace renderer
