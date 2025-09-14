#ifndef COMMAND_BUFFER_VULKAN_HPP
#define COMMAND_BUFFER_VULKAN_HPP

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/command_buffer/inc/command_buffer.hpp"

namespace renderer {
namespace image {
class ImageVulkan;
}   // namespace image

namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace command_buffer {
class CommandBufferVulkan : public CommandBuffer {
public:
    CommandBufferVulkan(
        rendering_device::RenderingDeviceVulkan* f_parentDevice,
        vk::raii::CommandPool&                   f_commandPool
    );

    CommandBufferVulkan& setBufferCount(uint32_t f_count);
    CommandBufferVulkan& setRendering(bool f_rendering);
    CommandBufferVulkan& create();

    CommandBufferVulkan& submit() override final;
    CommandBufferVulkan& reset() override final;
    CommandBufferVulkan& begin() override final;
    CommandBufferVulkan& end() override final;
    CommandBufferVulkan& beginRendering(
        const RenderBeginInfo& f_renderBeginInfo
    ) override final;
    CommandBufferVulkan& endRendering() override final;

    CommandBufferVulkan& transitionImageLayout(
        image::ImageVulkan*     f_image,
        vk::ImageLayout         f_old_layout,
        vk::ImageLayout         f_new_layout,
        vk::AccessFlags2        f_src_access_mask,
        vk::AccessFlags2        f_dst_access_mask,
        vk::PipelineStageFlags2 f_src_stage_mask,
        vk::PipelineStageFlags2 f_dst_stage_mask
    );

    vk::raii::CommandBuffers& getNativeHandle();

protected:
    void                     createCommandBuffer();
    vk::raii::CommandBuffer& selectCurrentCommandBuffer();

    uint32_t m_bufferCount{ 1 };

    bool m_rendering{ false };

    std::shared_ptr<render_target::RenderTarget> m_currentRenderTarget{ nullptr };

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };
    vk::raii::CommandBuffers                 m_commandBuffers{ nullptr };
    vk::raii::CommandPool&                   m_commandPool;
};
}   // namespace command_buffer
}   // namespace  renderer


#endif
