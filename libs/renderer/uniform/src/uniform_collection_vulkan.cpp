#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/texture/inc/texture_vulkan.hpp"
#include "renderer/uniform/inc/uniform_single_vulkan.hpp"

namespace renderer {
namespace uniform {
UniformCollectionVulkan::UniformCollectionVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : UniformCollection(),
      m_parentDevice(f_parentDevice)
{}

void UniformCollectionVulkan::update()
{
    m_uniformBuffers[m_parentDevice->getCurrentFrame()].upload(
        createStd140Buffer().data(), m_layout.m_structSize
    );
}

size_t UniformCollectionVulkan::getSize() const
{
    return m_layout.m_structSize;
}

size_t UniformCollectionVulkan::getAlignment() const
{
    return 16;   // Default memory alignment for structs in vulkan
}

UniformSingle* UniformCollectionVulkan::addMember(const std::string& f_name)
{
    m_members.push_back(std::make_unique<UniformSingleVulkan>());
    m_members.back()->setName(f_name);
    return dynamic_cast<UniformSingle*>(m_members.back().get());
}

UniformCollectionVulkan& UniformCollectionVulkan::setShaderstage(
    vk::ShaderStageFlagBits f_shaderStage
)
{
    m_shaderStage = f_shaderStage;
    return *this;
}

UniformCollectionVulkan& UniformCollectionVulkan::create()
{
    Uniform::create();
    createDescriptorSetLayout();
    computeStd140Layout();
    createUniformBuffers();
    createDescriptorSets();
    return *this;
}

vk::DescriptorSetLayout UniformCollectionVulkan::getDescriptorSetLayout() const
{
    return *m_descriptorSetLayout;
}

vk::raii::DescriptorSet& UniformCollectionVulkan::getDescriptorSet()
{
    return m_descriptorSets[m_parentDevice->getCurrentFrame()];
}

namespace {
inline vk::ShaderStageFlags convertPipelineToShaderStage(
    vk::PipelineStageFlags2 f_pipelineStages
)
{
    vk::ShaderStageFlags shaderStages{};

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eVertexShader) {
        shaderStages |= vk::ShaderStageFlagBits::eVertex;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eTessellationControlShader) {
        shaderStages |= vk::ShaderStageFlagBits::eTessellationControl;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eTessellationEvaluationShader) {
        shaderStages |= vk::ShaderStageFlagBits::eTessellationEvaluation;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eGeometryShader) {
        shaderStages |= vk::ShaderStageFlagBits::eGeometry;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eFragmentShader) {
        shaderStages |= vk::ShaderStageFlagBits::eFragment;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eComputeShader) {
        shaderStages |= vk::ShaderStageFlagBits::eCompute;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eTaskShaderEXT) {
        shaderStages |= vk::ShaderStageFlagBits::eTaskEXT;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eMeshShaderEXT) {
        shaderStages |= vk::ShaderStageFlagBits::eMeshEXT;
    }

    if (f_pipelineStages & vk::PipelineStageFlagBits2::eRayTracingShaderKHR) {
        // Vulkan groups all ray tracing stages under this pipeline flag.
        // To be safe, expose all ray tracing shader stages.
        shaderStages |= vk::ShaderStageFlagBits::eRaygenKHR
                      | vk::ShaderStageFlagBits::eAnyHitKHR
                      | vk::ShaderStageFlagBits::eClosestHitKHR
                      | vk::ShaderStageFlagBits::eMissKHR
                      | vk::ShaderStageFlagBits::eIntersectionKHR
                      | vk::ShaderStageFlagBits::eCallableKHR;
    }

    return shaderStages;
}
}   // namespace

void UniformCollectionVulkan::createDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> l_layoutBindings;

    l_layoutBindings.push_back(
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1, m_shaderStage, nullptr
        )
    );

    for (int i = 1; i <= m_textures.size(); i++) {
        texture::TextureVulkan* l_rawVulkanTexture =
            dynamic_cast<texture::TextureVulkan*>(m_textures[i - 1].get());

        if (l_rawVulkanTexture == nullptr) {
            throw std::
                runtime_error(
                    "UniformCollectionVulkan::createDescriptorSetLayout() texture isn't "
                    "a vulkan texture"
                );
        }

        l_layoutBindings.push_back(
            vk::DescriptorSetLayoutBinding(
                i,
                vk::DescriptorType::eCombinedImageSampler,
                1,
                convertPipelineToShaderStage(
                    l_rawVulkanTexture->getShaderStageDestination()
                ),
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

namespace {
inline size_t alignUp(size_t f_offset, size_t f_alignment)
{
    return (f_offset + f_alignment - 1) & ~(f_alignment - 1);
}
}   // namespace

void UniformCollectionVulkan::createUniformBuffers()
{
    m_uniformBuffers.clear();
    for (int i = 0; i < m_parentDevice->getMaxFramesInFlight(); i++) {
        m_bufferSize = m_layout.m_structSize;
        m_uniformBuffers.push_back(
            utils::VmaBuffer(
                m_parentDevice->getVmaAllocator(),
                m_bufferSize,
                vk::BufferUsageFlagBits::eUniformBuffer
                    | vk::BufferUsageFlagBits::eTransferDst,
                VMA_MEMORY_USAGE_CPU_TO_GPU
            )
        );
    }
}

void UniformCollectionVulkan::createDescriptorSets()
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
        vk::DescriptorBufferInfo l_bufferInfo{ .buffer = m_uniformBuffers[i].get(),
                                               .offset = 0,
                                               .range  = m_layout.m_structSize };

        std::vector<vk::WriteDescriptorSet> l_descriptorWrites;
        l_descriptorWrites.push_back(
            vk::WriteDescriptorSet{ .dstSet          = m_descriptorSets[i],
                                    .dstBinding      = 0,
                                    .dstArrayElement = 0,
                                    .descriptorCount = 1,
                                    .descriptorType  = vk::DescriptorType::eUniformBuffer,
                                    .pBufferInfo     = &l_bufferInfo }
        );

        for (int j = 0; j < m_textures.size(); j++) {
            texture::TextureVulkan* l_rawVulkanTexture =
                dynamic_cast<texture::TextureVulkan*>(m_textures[j].get());

            if (l_rawVulkanTexture == nullptr) {
                throw std::runtime_error(
                    "UniformCollectionVulkan::createDescriptorSets() texture isn't a "
                    "vulkan texture"
                );
            }

            vk::DescriptorImageInfo l_imageInfo(
                l_rawVulkanTexture->getSampler(),
                l_rawVulkanTexture->getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal
            );

            l_descriptorWrites.push_back(
                vk::WriteDescriptorSet{
                    .dstSet          = m_descriptorSets[i],
                    .dstBinding      = static_cast<uint32_t>(j + 1),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo      = &l_imageInfo }
            );
        }

        m_parentDevice->getLogicalDevice().updateDescriptorSets(l_descriptorWrites, {});
    }
}

void UniformCollectionVulkan::computeStd140Layout()
{
    size_t l_offset   = 0;
    size_t l_maxAlign = 16;   // std140 requires struct alignment at least 16

    m_layout.m_offsets.clear();
    for (auto& l_member : m_members) {
        size_t l_align = l_member->getAlignment();
        size_t l_size  = l_member->getSize();

        l_maxAlign = std::max(l_maxAlign, l_align);
        l_offset   = alignUp(l_offset, l_align);
        m_layout.m_offsets.push_back(l_offset);
        l_offset += l_size;
    }

    m_layout.m_structSize = alignUp(l_offset, l_maxAlign);
}

std::vector<uint8_t> UniformCollectionVulkan::createStd140Buffer()
{
    std::vector<uint8_t> l_buffer(m_layout.m_structSize, 0);

    for (size_t i = 0; i < m_members.size(); i++) {
        if (m_members[i]->getType() != UniformType::UNIFORM_TYPE_STRUCT) {
            uniform::UniformSingle* l_rawMember =
                dynamic_cast<uniform::UniformSingle*>(m_members[i].get());
            std::memcpy(
                l_buffer.data() + m_layout.m_offsets[i],
                l_rawMember->valueAsVoid(),
                m_members[i]->getSize()
            );
        }
    }

    return l_buffer;
}

}   // namespace uniform
}   // namespace renderer
