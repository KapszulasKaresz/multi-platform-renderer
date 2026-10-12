#ifndef MATERIAL_VULKAN_HPP_INCLUDED
#define MATERIAL_VULKAN_HPP_INCLUDED

#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/material/inc/material.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace image {
class ImageVulkan;
}   // namespace image

namespace material {
class MaterialVulkan : public Material {
public:
    MaterialVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    MaterialVulkan& create();

    vk::Pipeline                     getPipeline();
    vk::PipelineLayout               getPipelineLayout();
    std::vector<vk::DescriptorSet>   getDescriptorSets();
    std::vector<image::ImageVulkan*> getImages();

    vk::PipelineBindPoint getPipelineBindPoint() const;
    vk::StridedDeviceAddressRegionKHR getRaygenRegion() const;
    vk::StridedDeviceAddressRegionKHR getMissRegion() const;
    vk::StridedDeviceAddressRegionKHR getHitRegion() const;
    void setRayTracingOutput(image::ImageVulkan* f_image);

private:
    void createGraphicsPipeline();
    void createComputePipeline();
    void createRayTracingPipeline();

    [[nodiscard]]
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& f_code) const;

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::PipelineLayout m_pipelineLayout{ nullptr };
    vk::raii::Pipeline       m_pipeline{ nullptr };
    utils::VmaBuffer          m_shaderBindingTable{};
    vk::DeviceSize            m_shaderGroupHandleSize{ 0 };
    vk::DeviceSize            m_shaderGroupBaseAlignment{ 0 };
    vk::raii::DescriptorSetLayout m_rayTracingOutputLayout{ nullptr };
    std::vector<vk::raii::DescriptorSet> m_rayTracingOutputSets{};
};
}   // namespace material
}   // namespace  renderer


#endif
