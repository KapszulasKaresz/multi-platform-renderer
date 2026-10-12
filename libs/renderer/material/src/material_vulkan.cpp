#include "renderer/material/inc/material_vulkan.hpp"

#include <vector>
#include <cstring>

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/uniform/inc/uniform_array_vulkan.hpp"
#include "renderer/uniform/inc/uniform_acceleration_structure_vulkan.hpp"
#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer_vulkan.hpp"
#include "renderer/utils/inc/utils.hpp"
#include "renderer/texture/inc/texture_vulkan.hpp"

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

    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RENDER) {
        createGraphicsPipeline();
    }
    else if (getMaterialType() == MaterialType::MATERIAL_TYPE_COMPUTE) {
        createComputePipeline();
    }
    else if (getMaterialType() == MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        createRayTracingPipeline();
    }
    else {
        throw std::runtime_error("MaterialVulkan::create() Unsupported material type");
    }

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
        return m_pipeline;
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

    for (const auto& l_storageBuffer : m_uniformStorageBuffers) {
        uniform::UniformStorageBufferVulkan* l_rawStorageBuffer =
            dynamic_cast<uniform::UniformStorageBufferVulkan*>(l_storageBuffer.get());

        if (l_rawStorageBuffer != nullptr) {
            l_ret.push_back(l_rawStorageBuffer->getDescriptorSet());
        }
    }

    for (const auto& l_accelerationStructure : m_uniformAccelerationStructures) {
        auto* l_rawUniform =
            dynamic_cast<uniform::UniformAccelerationStructureVulkan*>(
                l_accelerationStructure.get()
            );
        if (l_rawUniform != nullptr) {
            l_ret.push_back(l_rawUniform->getDescriptorSet());
        }
    }
    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        l_ret.push_back(m_rayTracingAlbedoSets[m_parentDevice->getCurrentFrame()]);
        l_ret.push_back(m_rayTracingNormalSets[m_parentDevice->getCurrentFrame()]);
        l_ret.push_back(
            m_rayTracingMetallicRoughnessSets[m_parentDevice->getCurrentFrame()]
        );
        l_ret.push_back(m_rayTracingOutputSets[m_parentDevice->getCurrentFrame()]);
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

vk::PipelineBindPoint MaterialVulkan::getPipelineBindPoint() const
{
    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RENDER) {
        return vk::PipelineBindPoint::eGraphics;
    }
    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        return vk::PipelineBindPoint::eRayTracingKHR;
    }
    return vk::PipelineBindPoint::eCompute;
}

void MaterialVulkan::createGraphicsPipeline()
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

    for (const auto& l_storageBuffer : m_uniformStorageBuffers) {
        uniform::UniformStorageBufferVulkan* l_rawStorageBuffer =
            dynamic_cast<uniform::UniformStorageBufferVulkan*>(l_storageBuffer.get());

        if (l_rawStorageBuffer != nullptr) {
            l_descriptorSetLayouts.push_back(l_rawStorageBuffer->getDescriptorSetLayout());
        }

        for (const auto& l_accelerationStructure : m_uniformAccelerationStructures) {
            auto* l_rawUniform =
                dynamic_cast<uniform::UniformAccelerationStructureVulkan*>(
                    l_accelerationStructure.get()
                );
            if (l_rawUniform != nullptr) {
                l_descriptorSetLayouts.push_back(l_rawUniform->getDescriptorSetLayout());
            }
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

    m_pipeline =
        vk::raii::Pipeline(m_parentDevice->getLogicalDevice(), nullptr, l_pipelineInfo);
}

void MaterialVulkan::createComputePipeline()
{
    vk::raii::ShaderModule l_shaderModule =
        createShaderModule(utils::readFile(m_shaderLocation + ".comp.spv"));

    vk::PipelineShaderStageCreateInfo l_computeStageInfo{
        .stage  = vk::ShaderStageFlagBits::eCompute,
        .module = l_shaderModule,
        .pName  = "computeMain"
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

    for (const auto& l_storageBuffer : m_uniformStorageBuffers) {
        uniform::UniformStorageBufferVulkan* l_rawStorageBuffer =
            dynamic_cast<uniform::UniformStorageBufferVulkan*>(l_storageBuffer.get());

        if (l_rawStorageBuffer != nullptr) {
            l_descriptorSetLayouts.push_back(l_rawStorageBuffer->getDescriptorSetLayout());
        }

        for (const auto& l_accelerationStructure : m_uniformAccelerationStructures) {
            auto* l_rawUniform =
                dynamic_cast<uniform::UniformAccelerationStructureVulkan*>(
                    l_accelerationStructure.get()
                );
            if (l_rawUniform != nullptr) {
                l_descriptorSetLayouts.push_back(l_rawUniform->getDescriptorSetLayout());
            }
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

    vk::ComputePipelineCreateInfo l_pipelineInfo{ .stage  = l_computeStageInfo,
                                                  .layout = *m_pipelineLayout };

    m_pipeline =
        vk::raii::Pipeline(m_parentDevice->getLogicalDevice(), nullptr, l_pipelineInfo);
}

void MaterialVulkan::createRayTracingPipeline()
{
    const auto l_raygenModule =
        createShaderModule(utils::readFile(m_shaderLocation + ".rgen.spv"));
    const auto l_missModule =
        createShaderModule(utils::readFile(m_shaderLocation + ".rmiss.spv"));
    const auto l_closestHitModule =
        createShaderModule(utils::readFile(m_shaderLocation + ".rchit.spv"));

    std::array l_stages = {
        vk::PipelineShaderStageCreateInfo{
            .stage  = vk::ShaderStageFlagBits::eRaygenKHR,
            .module = l_raygenModule,
            .pName  = "raygenMain"
        },
        vk::PipelineShaderStageCreateInfo{
            .stage  = vk::ShaderStageFlagBits::eMissKHR,
            .module = l_missModule,
            .pName  = "missMain"
        },
        vk::PipelineShaderStageCreateInfo{
            .stage  = vk::ShaderStageFlagBits::eClosestHitKHR,
            .module = l_closestHitModule,
            .pName  = "closestHitMain"
        }
    };
    std::array l_groups = {
        vk::RayTracingShaderGroupCreateInfoKHR{
            .type              = vk::RayTracingShaderGroupTypeKHR::eGeneral,
            .generalShader     = 0,
            .closestHitShader  = VK_SHADER_UNUSED_KHR,
            .anyHitShader      = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR
        },
        vk::RayTracingShaderGroupCreateInfoKHR{
            .type              = vk::RayTracingShaderGroupTypeKHR::eGeneral,
            .generalShader     = 1,
            .closestHitShader  = VK_SHADER_UNUSED_KHR,
            .anyHitShader      = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR
        },
        vk::RayTracingShaderGroupCreateInfoKHR{
            .type              = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
            .generalShader     = VK_SHADER_UNUSED_KHR,
            .closestHitShader  = 2,
            .anyHitShader      = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR
        }
    };

    const vk::DescriptorSetLayoutBinding l_outputBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageImage,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR
    };
    m_rayTracingOutputLayout = vk::raii::DescriptorSetLayout(
        m_parentDevice->getLogicalDevice(),
        { .bindingCount = 1, .pBindings = &l_outputBinding }
    );
    std::vector<vk::DescriptorSetLayout> l_outputLayouts(
        m_parentDevice->getMaxFramesInFlight(), *m_rayTracingOutputLayout
    );
    m_rayTracingOutputSets = m_parentDevice->getLogicalDevice().allocateDescriptorSets(
        { .descriptorPool = m_parentDevice->getDescriptorPool(),
          .descriptorSetCount = static_cast<uint32_t>(l_outputLayouts.size()),
          .pSetLayouts = l_outputLayouts.data() }
    );

    const uint32_t l_textureCount =
        static_cast<uint32_t>(m_rayTracingAlbedoTextures.size());
    if (l_textureCount == 0) {
        throw std::runtime_error(
            "MaterialVulkan::createRayTracingPipeline() requires albedo textures"
        );
    }
    const vk::DescriptorSetLayoutBinding l_albedoBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = l_textureCount,
            .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR
    };
    m_rayTracingAlbedoLayout = vk::raii::DescriptorSetLayout(
        m_parentDevice->getLogicalDevice(),
        { .bindingCount = 1, .pBindings = &l_albedoBinding }
    );
    const auto createTextureLayout =
        [&](uint32_t f_count) {
            const vk::DescriptorSetLayoutBinding l_binding{
                .binding = 0,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = f_count,
                .stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR
            };
            return vk::raii::DescriptorSetLayout(
                m_parentDevice->getLogicalDevice(),
                { .bindingCount = 1, .pBindings = &l_binding }
            );
        };
    if (m_rayTracingNormalTextures.size() != l_textureCount
        || m_rayTracingMetallicRoughnessTextures.size() != l_textureCount)
    {
        throw std::runtime_error(
            "MaterialVulkan::createRayTracingPipeline() texture arrays are inconsistent"
        );
    }
    m_rayTracingNormalLayout = createTextureLayout(l_textureCount);
    m_rayTracingMetallicRoughnessLayout = createTextureLayout(l_textureCount);

    std::vector<vk::DescriptorSetLayout> l_descriptorSetLayouts;
    for (const auto& l_uniform : m_uniformCollections) {
        if (auto* l_raw = dynamic_cast<uniform::UniformCollectionVulkan*>(l_uniform.get())) {
            l_descriptorSetLayouts.push_back(l_raw->getDescriptorSetLayout());
        }
    }
    for (const auto& l_uniform : m_uniformArrays) {
        if (auto* l_raw = dynamic_cast<uniform::UniformArrayVulkan*>(l_uniform.get())) {
            l_descriptorSetLayouts.push_back(l_raw->getDescriptorSetLayout());
        }
    }
    for (const auto& l_uniform : m_uniformStorageBuffers) {
        if (auto* l_raw =
                dynamic_cast<uniform::UniformStorageBufferVulkan*>(l_uniform.get()))
        {
            l_descriptorSetLayouts.push_back(l_raw->getDescriptorSetLayout());
        }
    }
    for (const auto& l_uniform : m_uniformAccelerationStructures) {
        if (auto* l_raw = dynamic_cast<uniform::UniformAccelerationStructureVulkan*>(
                l_uniform.get()
            ))
        {
            l_descriptorSetLayouts.push_back(l_raw->getDescriptorSetLayout());
        }
    }
    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        l_descriptorSetLayouts.push_back(*m_rayTracingAlbedoLayout);
        l_descriptorSetLayouts.push_back(*m_rayTracingNormalLayout);
        l_descriptorSetLayouts.push_back(*m_rayTracingMetallicRoughnessLayout);
        l_descriptorSetLayouts.push_back(*m_rayTracingOutputLayout);
    }

    m_pipelineLayout = vk::raii::PipelineLayout(
        m_parentDevice->getLogicalDevice(),
        { .setLayoutCount = static_cast<uint32_t>(l_descriptorSetLayouts.size()),
          .pSetLayouts    = l_descriptorSetLayouts.data() }
    );

    const vk::RayTracingPipelineCreateInfoKHR l_pipelineInfo{
        .stageCount             = static_cast<uint32_t>(l_stages.size()),
        .pStages                = l_stages.data(),
        .groupCount             = static_cast<uint32_t>(l_groups.size()),
        .pGroups                = l_groups.data(),
        .maxPipelineRayRecursionDepth = 1,
        .layout                 = *m_pipelineLayout
    };
    m_pipeline = m_parentDevice->getLogicalDevice().createRayTracingPipelineKHR(
        nullptr, nullptr, l_pipelineInfo
    );

    auto l_properties = m_parentDevice->getPhysicalDevice().getProperties2<
        vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR
    >();
    const auto& l_rayTracingProperties =
        l_properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
    m_shaderGroupHandleSize = l_rayTracingProperties.shaderGroupHandleSize;
    m_shaderGroupBaseAlignment = l_rayTracingProperties.shaderGroupBaseAlignment;
    const vk::DeviceSize l_stride = m_shaderGroupBaseAlignment;
    const vk::DeviceSize l_size = l_stride * l_stages.size();
    auto l_handles = m_pipeline.getRayTracingShaderGroupHandlesKHR<uint8_t>(
        0, static_cast<uint32_t>(l_stages.size()),
        static_cast<size_t>(m_shaderGroupHandleSize * l_stages.size())
    );
    m_shaderBindingTable = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(), l_size,
        vk::BufferUsageFlagBits::eShaderBindingTableKHR
            | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );
    std::vector<uint8_t> l_sbtData(l_size, 0);
    for (size_t i = 0; i < l_stages.size(); ++i) {
        std::memcpy(
            l_sbtData.data() + i * l_stride,
            l_handles.data() + i * m_shaderGroupHandleSize,
            static_cast<size_t>(m_shaderGroupHandleSize)
        );
    }

    m_shaderBindingTable.upload(l_sbtData.data(), l_sbtData.size());

    std::vector<vk::DescriptorSetLayout> l_albedoLayouts(
        m_parentDevice->getMaxFramesInFlight(), *m_rayTracingAlbedoLayout
    );
    m_rayTracingAlbedoSets = m_parentDevice->getLogicalDevice().allocateDescriptorSets(
        { .descriptorPool = m_parentDevice->getDescriptorPool(),
          .descriptorSetCount = static_cast<uint32_t>(l_albedoLayouts.size()),
          .pSetLayouts = l_albedoLayouts.data() }
    );
    const auto createTextureSets =
        [&](const auto& f_textures, auto& f_layout, auto& f_sets) {
        std::vector<vk::DescriptorSetLayout> l_layouts(
            m_parentDevice->getMaxFramesInFlight(), *f_layout
        );
        f_sets = m_parentDevice->getLogicalDevice().allocateDescriptorSets(
            { .descriptorPool = m_parentDevice->getDescriptorPool(),
              .descriptorSetCount = static_cast<uint32_t>(l_layouts.size()),
              .pSetLayouts = l_layouts.data() }
        );
        for (auto& l_set : f_sets) {
            for (uint32_t i = 0; i < l_textureCount; ++i) {
                auto* l_texture =
                    dynamic_cast<texture::TextureVulkan*>(f_textures[i].get());
            if (!l_texture) {
                throw std::runtime_error(
                    "MaterialVulkan::createRayTracingPipeline() invalid ray-tracing texture"
                );
            }
            const vk::DescriptorImageInfo l_imageInfo{
                .sampler = l_texture->getSampler(),
                .imageView = l_texture->getImageView(),
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            };
            m_parentDevice->getLogicalDevice().updateDescriptorSets(
                { vk::WriteDescriptorSet{
                    .dstSet = l_set,
                    .dstBinding = 0,
                    .dstArrayElement = i,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &l_imageInfo
                } },
                {}
            );
            }
        }
    };
    createTextureSets(
        m_rayTracingAlbedoTextures, m_rayTracingAlbedoLayout, m_rayTracingAlbedoSets
    );
    createTextureSets(
        m_rayTracingNormalTextures, m_rayTracingNormalLayout, m_rayTracingNormalSets
    );
    createTextureSets(
        m_rayTracingMetallicRoughnessTextures,
        m_rayTracingMetallicRoughnessLayout,
        m_rayTracingMetallicRoughnessSets
    );
}

vk::StridedDeviceAddressRegionKHR MaterialVulkan::getRaygenRegion() const
{
    const auto l_address = m_parentDevice->getLogicalDevice().getBufferAddress(
        { .buffer = m_shaderBindingTable.get() }
    );
    return { .deviceAddress = l_address,
             .stride       = m_shaderGroupBaseAlignment,
             .size         = m_shaderGroupBaseAlignment };
}

void MaterialVulkan::setRayTracingOutput(image::ImageVulkan* f_image)
{
    if (!f_image || m_rayTracingOutputSets.empty()) {
        throw std::runtime_error("MaterialVulkan::setRayTracingOutput() invalid image");
    }
    auto& l_set = m_rayTracingOutputSets[m_parentDevice->getCurrentFrame()];
    const vk::DescriptorImageInfo l_imageInfo{
        .sampler = nullptr,
        .imageView = f_image->getImageView(),
        .imageLayout = vk::ImageLayout::eGeneral
    };
    m_parentDevice->getLogicalDevice().updateDescriptorSets(
        { vk::WriteDescriptorSet{
            .dstSet = l_set,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageImage,
            .pImageInfo = &l_imageInfo
        } },
        {}
    );
}

vk::StridedDeviceAddressRegionKHR MaterialVulkan::getMissRegion() const
{
    const auto l_address = m_parentDevice->getLogicalDevice().getBufferAddress(
        { .buffer = m_shaderBindingTable.get() }
    );
    return { .deviceAddress = l_address + m_shaderGroupBaseAlignment,
             .stride       = m_shaderGroupBaseAlignment,
             .size         = m_shaderGroupBaseAlignment };
}

vk::StridedDeviceAddressRegionKHR MaterialVulkan::getHitRegion() const
{
    const auto l_address = m_parentDevice->getLogicalDevice().getBufferAddress(
        { .buffer = m_shaderBindingTable.get() }
    );
    return { .deviceAddress = l_address + 2 * m_shaderGroupBaseAlignment,
             .stride       = m_shaderGroupBaseAlignment,
             .size         = m_shaderGroupBaseAlignment };
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
