#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/material/inc/material_vulkan.hpp"
#include "renderer/mesh/inc/triangle_mesh_vulkan.hpp"
#include "renderer/render_target/inc/render_target.hpp"
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

CommandBufferVulkan& CommandBufferVulkan::setUsage(bool f_usage)
{
    m_singleUse = f_usage;
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
    if (m_singleUse && m_used) {
        throw std::runtime_error(
            "CommandBufferVulkan::submit() you want to resubmit a single use command "
            "buffer"
        );
    }
    if (m_rendering) {
        m_parentDevice->submitRenderCommandBuffer(this);
    }
    else {
        m_parentDevice->submitCommandBuffer(this);
    }
    m_used = true;
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


    l_commanBuffer.begin(
        m_singleUse
            ? vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::
                                              eOneTimeSubmit }
            : vk::CommandBufferBeginInfo{}
    );
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::end()
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.end();
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::beginRendering(
    const RenderBeginInfo& f_renderBeginInfo
)
{
    if (m_currentRenderTarget) {
        throw std::runtime_error(
            "CommandBufferVulkan::beginRendering(...) you need to end the previous "
            "rendering before you can start a new one"
        );
    }

    m_currentRenderTarget = f_renderBeginInfo.m_renderTarget;
    image::ImageVulkan* l_imageVulkan =
        dynamic_cast<image::ImageVulkan*>(m_currentRenderTarget->getImage().get());

    if (!l_imageVulkan) {
        throw std::runtime_error(
            "CommandBufferVulkan::beginRendering(...) render target didn't provide a "
            "vulkan image"
        );
    }

    transitionImageLayout(
        l_imageVulkan,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},   // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,          // dstAccessMask
        vk::PipelineStageFlagBits2::eTopOfPipe,              // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput   // dstStage
    );

    auto& l_commanBuffer = selectCurrentCommandBuffer();

    // Dummy stuff for now
    auto           l_clearColorVec = f_renderBeginInfo.m_clearColor;
    vk::ClearValue l_clearColor    = vk::ClearColorValue(
        l_clearColorVec.x, l_clearColorVec.y, l_clearColorVec.z, l_clearColorVec.w
    );
    vk::RenderingAttachmentInfo l_attachmentInfo = {
        .imageView   = l_imageVulkan->getImageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = l_clearColor
    };
    auto l_extentSize = m_currentRenderTarget->getSize();

    vk::RenderingInfo l_renderingInfo = {
        .renderArea           = { .offset = { 0, 0 },
                                 .extent = { static_cast<uint32_t>(l_extentSize.x),
                                              static_cast<uint32_t>(l_extentSize.y) } },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &l_attachmentInfo
    };
    l_commanBuffer.beginRendering(l_renderingInfo);

    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::endRendering()
{
    image::ImageVulkan* l_imageVulkan =
        dynamic_cast<image::ImageVulkan*>(m_currentRenderTarget->getImage().get());

    if (!l_imageVulkan) {
        throw std::runtime_error(
            "CommandBufferVulkan::endRendering(...) render target didn't provide a "
            "vulkan image"
        );
    }

    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.endRendering();
    transitionImageLayout(
        l_imageVulkan,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,           // srcAccessMask
        {},                                                   // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,   // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe             // dstStage
    );

    m_currentRenderTarget = nullptr;
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::useMaterial(
    std::shared_ptr<material::Material> f_material
)
{
    material::MaterialVulkan* l_vulkanMaterial =
        dynamic_cast<material::MaterialVulkan*>(f_material.get());

    if (!l_vulkanMaterial) {
        throw std::runtime_error(
            "CommandBufferVulkan::useMaterial(...) non vulkan material provided"
        );
    }
    l_vulkanMaterial->updateUniforms();

    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, l_vulkanMaterial->getPipeline()
    );

    l_commanBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        l_vulkanMaterial->getPipelineLayout(),
        0,
        l_vulkanMaterial->getDescriptorSets(),
        nullptr
    );
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::useViewport(const ViewportInfo& f_viewportInfo)
{
    glm::vec2 l_size = f_viewportInfo.m_size;

    if (f_viewportInfo.m_fullScreen) {
        if (!m_currentRenderTarget) {
            throw std::runtime_error(
                    "CommandBufferVulkan::useViewport(...) cannot use fullscreen "
                    "viewport without a render target bound"
                );
        }
        l_size = m_currentRenderTarget->getSize();
    }

    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.setViewport(
        0,
        vk::Viewport(
            f_viewportInfo.m_positions.x,
            f_viewportInfo.m_positions.y,
            l_size.x,
            l_size.y,
            0.0f,
            1.0f
        )
    );
    l_commanBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), { l_size.x, l_size.y }));
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::draw(std::shared_ptr<mesh::TriangleMesh> f_mesh)
{
    auto&                     l_commanBuffer = selectCurrentCommandBuffer();
    mesh::TriangleMeshVulkan* l_triangleMeshVulkan =
        dynamic_cast<mesh::TriangleMeshVulkan*>(f_mesh.get());

    if (!l_triangleMeshVulkan) {
        throw std::runtime_error(
            "CommandBufferVulkan::draw((...) the mesh wasn't created using a vulkan "
            "device"
        );
    }

    l_commanBuffer.bindVertexBuffers(
        0, vk::Buffer(l_triangleMeshVulkan->getVertexBuffer().get()), { 0 }
    );
    l_commanBuffer.bindIndexBuffer(
        vk::Buffer(l_triangleMeshVulkan->getIndexBuffer().get()), 0, vk::IndexType::eUint32
    );
    l_commanBuffer.drawIndexed(l_triangleMeshVulkan->getIndicieCount(), 1, 0, 0, 0);

    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::copyBuffer(
    utils::VmaBuffer& f_srcBuffer,
    utils::VmaBuffer& f_destBuffer,
    vk::DeviceSize    f_size
)
{
    auto& l_commanBuffer = selectCurrentCommandBuffer();
    l_commanBuffer.copyBuffer(
        f_srcBuffer.get(), f_destBuffer.get(), vk::BufferCopy(0, 0, f_size)
    );
    return *this;
}

CommandBufferVulkan& CommandBufferVulkan::copyBuffer(
    utils::VmaBuffer& f_srcBuffer,
    utils::VmaImage&  f_destImage,
    uint32_t          f_width,
    uint32_t          f_height
)
{
    auto&               l_commanBuffer = selectCurrentCommandBuffer();
    vk::BufferImageCopy l_region{
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
        .imageOffset       = { 0, 0, 0 },
        .imageExtent       = { f_width, f_height, 1 }
    };
    l_commanBuffer.copyBufferToImage(
        f_srcBuffer.get(),
        f_destImage.get(),
        vk::ImageLayout::eTransferDstOptimal,
        { l_region }
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
