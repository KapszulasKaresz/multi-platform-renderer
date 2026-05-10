#include "renderer/material/inc/material_vulkan.hpp"

#include <vector>

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/uniform/inc/uniform_array_vulkan.hpp"
#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"
#include "renderer/utils/inc/utils.hpp"

namespace renderer {
namespace material {
MaterialVulkan::MaterialVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice)
    : Material(),
      m_parentDevice(f_parentDevice)
{}

MaterialVulkan& MaterialVulkan::create()
{
    if (!isOriginal()) {
        m_valid = true;
        return *this;
    }
    createPipeline();

    for (const auto& image : getImages()) {
        if (image && image->getCurrentLayout() != vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
            auto l_commandBufferVulkan =
                dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

            l_commandBufferVulkan->begin();
            l_commandBufferVulkan->transitionImageLayout(
                image,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                {},
                vk::AccessFlagBits2::eShaderSampledRead,
                vk::PipelineStageFlagBits2::eTopOfPipe,
                vk::PipelineStageFlagBits2::eFragmentShader,
                vk::ImageAspectFlagBits::eColor
            );
            l_commandBufferVulkan->end();
            l_commandBufferVulkan->submit();
        }
    }

    m_valid = true;
    return *this;
}

vk::Pipeline MaterialVulkan::getPipeline()
{
    if (!isValid()) {
        throw std::runtime_error("Material pipeline is not valid");
    }
    if (isOriginal()) {
        return m_graphicsPipeline;
    }
    else {
        return dynamic_cast<MaterialVulkan*>(m_original.get())->getPipeline();
    }
}

vk::PipelineLayout MaterialVulkan::getPipelineLayout()
{
    if (!isValid()) {
        throw std::runtime_error("Material pipeline layout is not valid");
    }
    if (isOriginal()) {
        return m_pipelineLayout;
    }
    else {
        return dynamic_cast<MaterialVulkan*>(m_original.get())->getPipelineLayout();
    }
}

std::vector<vk::DescriptorSet> MaterialVulkan::getDescriptorSets()
{
    std::vector<vk::DescriptorSet> l_ret;
    for (const auto& l_uniform : m_uniformCollections) {
        uniform::UniformCollectionVulkan* l_rawCollection =
            dynamic_cast<uniform::UniformCollectionVulkan*>(l_uniform.get());

        if (l_rawCollection != nullptr) {
            l_ret.push_back(l_rawCollection->getDescriptorSet());
        }
    }

    for (const auto& l_array : m_uniformArrays) {
        uniform::UniformArrayVulkan* l_rawArray =
            dynamic_cast<uniform::UniformArrayVulkan*>(l_array.get());

        if (l_rawArray != nullptr) {
            l_ret.push_back(l_rawArray->getDescriptorSet());
        }
    }

    return l_ret;
}

std::vector<image::ImageVulkan*> MaterialVulkan::getImages()
{
    std::vector<image::ImageVulkan*> l_ret;
    for (const auto& l_uniform : m_uniformCollections) {
        uniform::UniformCollectionVulkan* l_rawCollection =
            dynamic_cast<uniform::UniformCollectionVulkan*>(l_uniform.get());

        if (l_rawCollection != nullptr) {
            for (int i = 0; i < l_rawCollection->getTextureCount(); i++) {
                image::ImageVulkan* l_image = dynamic_cast<image::ImageVulkan*>(
                    l_rawCollection->getTexture(i)->getImage()
                );

                if (l_image != nullptr) {
                    l_ret.push_back(l_image);
                }
            }
        }
    }

    return l_ret;
}

void MaterialVulkan::createPipeline()
{
    vk::raii::ShaderModule l_shaderModule =
        createShaderModule(utils::readFile(m_shaderLocation + ".spv"));

    vk::PipelineShaderStageCreateInfo l_vertShaderStageInfo{
        .stage  = vk::ShaderStageFlagBits::eVertex,
        .module = l_shaderModule,
        .pName  = "vertMain"
    };
    vk::PipelineShaderStageCreateInfo l_fragShaderStageInfo{
        .stage  = vk::ShaderStageFlagBits::eFragment,
        .module = l_shaderModule,
        .pName  = "fragMain"
    };
    vk::PipelineShaderStageCreateInfo l_shaderStages[] = { l_vertShaderStageInfo,
                                                           l_fragShaderStageInfo };

    auto l_bindingDescription    = mesh::Vertex::getBindingDescription();
    auto l_attributeDescriptions = mesh::Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo l_vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &l_bindingDescription,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(l_attributeDescriptions.size()),
        .pVertexAttributeDescriptions = l_attributeDescriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo l_inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    vk::PipelineViewportStateCreateInfo l_viewportState{ .viewportCount = 1,
                                                         .scissorCount  = 1 };

    vk::PipelineRasterizationStateCreateInfo l_rasterizer{
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable         = vk::False,
        .depthBiasSlopeFactor    = 1.0f,
        .lineWidth               = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo l_multisampling{
        .rasterizationSamples =
            static_cast<vk::SampleCountFlagBits>(m_parentDevice->getMaxMSAASamples()),
        .sampleShadingEnable = m_parentDevice->isSampleShadingEnabled(),
        .minSampleShading    = m_parentDevice->sampleShadingLevel()
    };

    vk::PipelineColorBlendAttachmentState l_colorBlendAttachment{
        .blendEnable    = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo l_colorBlending{ .logicOpEnable = vk::False,
                                                           .logicOp = vk::LogicOp::eCopy,
                                                           .attachmentCount = 1,
                                                           .pAttachments =
                                                               &l_colorBlendAttachment };

    std::vector l_dynamicStates = { vk::DynamicState::eViewport,
                                    vk::DynamicState::eScissor,
                                    vk::DynamicState::eDepthTestEnable };

    vk::PipelineDepthStencilStateCreateInfo l_depthStencil{
        .depthTestEnable       = vk::True,
        .depthWriteEnable      = vk::True,
        .depthCompareOp        = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable     = vk::False
    };

    vk::PipelineDynamicStateCreateInfo l_dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(l_dynamicStates.size()),
        .pDynamicStates    = l_dynamicStates.data()
    };

    std::vector<vk::DescriptorSetLayout> l_descriptorSetLayouts;
    for (const auto& l_uniform : m_uniformCollections) {
        uniform::UniformCollectionVulkan* l_rawCollection =
            dynamic_cast<uniform::UniformCollectionVulkan*>(l_uniform.get());

        if (l_rawCollection != nullptr) {
            l_descriptorSetLayouts.push_back(l_rawCollection->getDescriptorSetLayout());
        }
    }

    for (const auto& l_array : m_uniformArrays) {
        uniform::UniformArrayVulkan* l_rawArray =
            dynamic_cast<uniform::UniformArrayVulkan*>(l_array.get());

        if (l_rawArray != nullptr) {
            l_descriptorSetLayouts.push_back(l_rawArray->getDescriptorSetLayout());
        }
    }

    vk::PipelineLayoutCreateInfo l_pipelineLayoutInfo{
        .setLayoutCount         = static_cast<uint32_t>(l_descriptorSetLayouts.size()),
        .pSetLayouts            = l_descriptorSetLayouts.data(),
        .pushConstantRangeCount = 0
    };

    m_pipelineLayout = vk::raii::PipelineLayout(
        m_parentDevice->getLogicalDevice(), l_pipelineLayoutInfo
    );

    auto l_colorAttachmentFormat = m_parentDevice->getSwapchainSurfaceFormat();

    vk::PipelineRenderingCreateInfo l_pipelineRenderingCreateInfo{
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &l_colorAttachmentFormat,
        .depthAttachmentFormat =
            image::ImageVulkan::findDepthFormat(m_parentDevice->getPhysicalDevice())
    };
    vk::GraphicsPipelineCreateInfo l_pipelineInfo{
        .pNext               = &l_pipelineRenderingCreateInfo,
        .stageCount          = 2,
        .pStages             = l_shaderStages,
        .pVertexInputState   = &l_vertexInputInfo,
        .pInputAssemblyState = &l_inputAssembly,
        .pViewportState      = &l_viewportState,
        .pRasterizationState = &l_rasterizer,
        .pMultisampleState   = &l_multisampling,
        .pDepthStencilState  = &l_depthStencil,
        .pColorBlendState    = &l_colorBlending,
        .pDynamicState       = &l_dynamicState,
        .layout              = m_pipelineLayout,
        .renderPass          = nullptr
    };

    m_graphicsPipeline =
        vk::raii::Pipeline(m_parentDevice->getLogicalDevice(), nullptr, l_pipelineInfo);
}

[[nodiscard]]
vk::raii::ShaderModule MaterialVulkan::createShaderModule(
    const std::vector<char>& f_code
) const
{
    vk::ShaderModuleCreateInfo l_createInfo{
        .codeSize = f_code.size(),
        .pCode    = reinterpret_cast<const uint32_t*>(f_code.data())
    };
    vk::raii::ShaderModule l_shaderModule{ m_parentDevice->getLogicalDevice(),
                                           l_createInfo };

    return l_shaderModule;
}

}   // namespace material
}   // namespace renderer
