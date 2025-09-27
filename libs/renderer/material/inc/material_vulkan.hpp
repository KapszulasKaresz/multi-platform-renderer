#ifndef MATERIAL_VULKAN_HPP_INCLUDED
#define MATERIAL_VULKAN_HPP_INCLUDED

#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace material {
class MaterialVulkan : public Material {
public:
    MaterialVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    MaterialVulkan& create();

    vk::Pipeline                   getPipeline();
    vk::PipelineLayout             getPipelineLayout();
    std::vector<vk::DescriptorSet> getDescriptorSets();

    void updateUniforms();

private:
    void createPipeline();

    [[nodiscard]]
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& f_code) const;

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::PipelineLayout m_pipelineLayout{ nullptr };
    vk::raii::Pipeline       m_graphicsPipeline{ nullptr };
};
}   // namespace material
}   // namespace  renderer


#endif
