#include "renderer/material/inc/material_vulkan.hpp"

#include <vector>

#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
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
    createPipeline();
    m_valid = true;
    return *this;
}

vk::Pipeline MaterialVulkan::getPipeline()
{
    return m_graphicsPipeline;
}

void MaterialVulkan::createPipeline()
{
    vk::raii::ShaderModule l_shaderModule =
        createShaderModule(utils::readFile(m_shaderLocation));

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
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .depthBiasSlopeFactor    = 1.0f,
        .lineWidth               = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo l_multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable  = vk::False
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
                                    vk::DynamicState::eScissor };

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
        .colorAttachmentCount = 1, .pColorAttachmentFormats = &l_colorAttachmentFormat
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
