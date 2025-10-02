#include "renderer/render_target/inc/render_target_vulkan.hpp"

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace render_target {

RenderTargetVulkan::RenderTargetVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

RenderTargetVulkan& RenderTargetVulkan::create()
{
    createColorResources();
    if (m_useDepthBuffer) {
        createDepthResources();
    }
    m_valid = true;
    return *this;
}

std::shared_ptr<image::Image> RenderTargetVulkan::getImage()
{
    return m_colorImage;
}

std::shared_ptr<image::Image> RenderTargetVulkan::getDepthImage()
{
    return m_depthImage;
}

glm::ivec2 RenderTargetVulkan::getSize() const
{
    return m_size;
}

void RenderTargetVulkan::createColorResources()
{
    m_colorImage =
        std::dynamic_pointer_cast<image::ImageVulkan>(m_parentDevice->createImage());

    m_colorImage->setFormat(m_format)
        .setColorSpace(m_colorSpace)
        .setSize(m_size)
        .setSampleCount(m_parentDevice->getMaxMSAASamples())
        .setUsage(
            vk::ImageUsageFlagBits::eTransientAttachment
            | vk::ImageUsageFlagBits::eColorAttachment
        )
        .createEmptyImage();
}

void RenderTargetVulkan::createDepthResources()
{
    m_depthImage =
        std::dynamic_pointer_cast<image::ImageVulkan>(m_parentDevice->createImage());
    m_depthImage->setFormat(image::IMAGE_FORMAT_DEPTH)
        .setColorSpace(image::COLOR_SPACE_LINEAR)
        .setSize(glm::ivec2(m_size))
        .setSampleCount(m_parentDevice->getMaxMSAASamples())
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .createEmptyImage();
}
}   // namespace render_target
}   // namespace renderer
