#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace command_buffer {
CommandBufferVulkan::CommandBufferVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice,
    vk::raii::CommandPool&                   f_commandPool
)
    : CommandBuffer(),
      m_parentDevice(f_parentDevice),
      m_commandPool(f_commandPool)
{}

CommandBufferVulkan& CommandBufferVulkan::setBufferCount(uint32_t f_count)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "CommandBufferVulkan::setBufferCount(int f_count) you can only set "
                "buffer " "count on uncreated buffer"
            );
    }
    m_bufferCount = f_count;
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::setRendering(bool f_rendering)
{
    m_rendering = f_rendering;
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::create()
{
    createCommandBuffer();
    m_valid = true;
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::submit()
{
    if (m_rendering) {
        m_parentDevice->submitRenderCommandBuffer(this);
    }
    else {
        m_parentDevice->submitCommandBuffer(this);
    }
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::reset()
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.reset();
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::begin()
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.begin({});
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::end()
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.end();
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::beginRendering()
{
    transitionImageLayout(
        m_parentDevice->getCurrentSwapChainImage().get(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},   // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,          // dstAccessMask
        vk::PipelineStageFlagBits2::eTopOfPipe,              // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput   // dstStage
    );

    auto& l_commanBuffer = selectCurrentCommandBuffer();

    // Dummy stuff for now
    vk::ClearValue l_clearColor = vk::ClearColorValue(1.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo l_attachmentInfo = {
        .imageView   = m_parentDevice->getCurrentSwapChainImage()->getImageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = l_clearColor
    };
    vk::RenderingInfo l_renderingInfo = {
        .renderArea           = { .offset = { 0, 0 }, .extent = { 100, 100 } },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &l_attachmentInfo
    };
    l_commanBuffer.beginRendering(l_renderingInfo);

    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::endRendering()
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.endRendering();
    transitionImageLayout(
        m_parentDevice->getCurrentSwapChainImage().get(),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,           // srcAccessMask
        {},                                                   // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,   // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe             // dstStage
    );
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::transitionImageLayout(
    image::ImageVulkan*     f_image,
    vk::ImageLayout         f_old_layout,
    vk::ImageLayout         f_new_layout,
    vk::AccessFlags2        f_src_access_mask,
    vk::AccessFlags2        f_dst_access_mask,
    vk::PipelineStageFlags2 f_src_stage_mask,
    vk::PipelineStageFlags2 f_dst_stage_mask
)
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();

    // TODO once there are images with mip levels extend this function
    vk::ImageMemoryBarrier2 l_barrier = {
        .srcStageMask        = f_src_stage_mask,
        .srcAccessMask       = f_src_access_mask,
        .dstStageMask        = f_dst_stage_mask,
        .dstAccessMask       = f_dst_access_mask,
        .oldLayout           = f_old_layout,
        .newLayout           = f_new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = f_image->getImage(),
        .subresourceRange    = { .aspectMask     = vk::ImageAspectFlagBits::eColor,
                                .baseMipLevel   = 0,
                                .levelCount     = 1,
                                .baseArrayLayer = 0,
                                .layerCount     = 1 }
    };
    vk::DependencyInfo l_dependency_info = { .dependencyFlags         = {},
                                             .imageMemoryBarrierCount = 1,
                                             .pImageMemoryBarriers    = &l_barrier };
    l_commanBuffer.pipelineBarrier2(l_dependency_info);
    return *this;
}

vk::raii::CommandBuffers& CommandBufferVulkan::getNativeHandle()
{
    return m_commandBuffers;
}

void CommandBufferVulkan::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo l_allocInfo{ .commandPool = m_commandPool,
                                               .level = vk::CommandBufferLevel::ePrimary,
                                               .commandBufferCount = m_bufferCount };
    m_commandBuffers =
        vk::raii::CommandBuffers(m_parentDevice->getLogicalDevice(), l_allocInfo);
}

vk::raii::CommandBuffer& CommandBufferVulkan::selectCurrentCommandBuffer()
{
    if (m_rendering) {
        return m_commandBuffers[m_parentDevice->getCurrentFrame()];
    }
    else {
        return m_commandBuffers[0];
    }
}

}   // namespace command_buffer
}   // namespace renderer
