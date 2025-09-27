#include "renderer/image/inc/image_vulkan.hpp"

#include <stb_image.h>

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace image {
ImageVulkan::ImageVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice)
    : Image(),
      m_parentDevice(f_parentDevice)
{}

ImageVulkan& ImageVulkan::create()
{
    if (isValid()) {
        throw std::runtime_error(
            "ImageVulkan::create() you cannot create an already created target"
        );
    }

    m_valid = true;
    return *this;
}

ImageVulkan& ImageVulkan::createFromSwapChainImage(vk::Image f_swapChainImage)
{
    if (isValid()) {
        throw std::runtime_error(
            "ImageVulkan::createFromSwapChainImage(vk::Image f_swapChainImage) you "
            "cannot create an already created image"
        );
    }

    m_swapchainImage = f_swapChainImage;

    m_imageView = createImageView(*m_swapchainImage, vk::ImageAspectFlagBits::eColor);
    m_valid     = true;
    return *this;
}

ImageVulkan& ImageVulkan::createFromFile(std::string_view f_path)
{
    if (isValid()) {
        throw std::runtime_error(
            "ImageVulkan::createFromFile(std::string_view f_path) you "
            "cannot create an already created image"
        );
    }

    int      l_texWidth, l_texHeight, l_texChannels;
    stbi_uc* l_pixels = stbi_load(
        f_path.data(), &l_texWidth, &l_texHeight, &l_texChannels, STBI_rgb_alpha
    );
    vk::DeviceSize l_imageSize = l_texWidth * l_texHeight * 4;

    if (!l_texWidth) {
        throw std::runtime_error(
            "ImageVulkan::createFromFile(std::string_view f_path) failed to load image!"
        );
    }

    auto l_stagingBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        l_imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        0
    );

    l_stagingBuffer.upload(l_pixels, l_imageSize);

    stbi_image_free(l_pixels);


    m_format = IMAGE_FORMAT_RGBA8_SRGB;
    vk::ImageCreateInfo l_imageCreateInfo{
        .imageType   = vk::ImageType::e2D,
        .format      = convertToVkFormat(m_format),
        .extent      = { .width  = static_cast<uint32_t>(l_texWidth),
                        .height = static_cast<uint32_t>(l_texHeight),
                        .depth  = 1 },
        .mipLevels   = 1,
        .arrayLayers = 1,
        .samples     = vk::SampleCountFlagBits::e1,
        .tiling      = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
    };

    m_image = utils::VmaImage(
        m_parentDevice->getVmaAllocator(),
        l_imageCreateInfo,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );

    transitionImageLayout(
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal
    );

    copyBufferToImage(l_stagingBuffer, l_texWidth, l_texHeight);

    transitionImageLayout(
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal
    );

    m_valid = true;
    return *this;
}

ImageVulkan& ImageVulkan::setFormat(image::ImageFormat f_format)
{
    Image::setFormat(f_format);
    return *this;
}

ImageVulkan& ImageVulkan::setColorSpace(image::ColorSpace f_colorSpace)
{
    Image::setColorSpace(f_colorSpace);
    return *this;
}

vk::Format ImageVulkan::convertToVkFormat(const ImageFormat f_format)
{
    switch (f_format) {
        case IMAGE_FORMAT_RGBA8: {
            return vk::Format::eR8G8B8A8Unorm;
        }

        case IMAGE_FORMAT_RGBA8_SRGB: {
            return vk::Format::eR8G8B8A8Srgb;
        }

        case IMAGE_FORMAT_RGB8: {
            return vk::Format::eR8G8B8Unorm;
        }

        case IMAGE_FORMAT_RGB8_SRGB: {
            return vk::Format::eR8G8B8Srgb;
        }

        case IMAGE_FORMAT_BGRA8: {
            return vk::Format::eB8G8R8A8Unorm;
        }

        case IMAGE_FORMAT_BGRA8_SRGB: {
            return vk::Format::eB8G8R8A8Srgb;
        }

        case IMAGE_FORMAT_UNDEFINED:
        case IMAGE_FORMAT_MAX:
        default:                     {
            return vk::Format::eUndefined;
        }
    }
}

vk::ColorSpaceKHR ImageVulkan::convertToVkColorSpace(const ColorSpace f_colorSpace)
{
    switch (f_colorSpace) {
        case COLOR_SPACE_LINEAR: {
            return vk::ColorSpaceKHR::eExtendedSrgbLinearEXT;
        }

        case COLOR_SPACE_SRGB_NON_LINEAR: {
            return vk::ColorSpaceKHR::eSrgbNonlinear;
        }

        case COLOR_SPACE_MAX:
        default:              {
            return vk::ColorSpaceKHR::eSrgbNonlinear;   // safe fallback
        }
    }
}

vk::Image ImageVulkan::getImage()
{
    return m_swapchainImage.value_or(vk::Image(m_image.get()));
}

vk::ImageView ImageVulkan::getImageView()
{
    return m_imageView;
}

vk::raii::ImageView
    ImageVulkan::createImageView(vk::Image& f_image, vk::ImageAspectFlags f_aspectFlags)
{
    vk::ImageViewCreateInfo l_imageViewCreateInfo{
        .image            = f_image,
        .viewType         = vk::ImageViewType::e2D,
        .format           = convertToVkFormat(m_format),
        .subresourceRange = { f_aspectFlags, 0, 1, 0, 1 }
    };

    return vk::raii::ImageView(m_parentDevice->getLogicalDevice(), l_imageViewCreateInfo);
}

void ImageVulkan::transitionImageLayout(
    vk::ImageLayout f_oldLayout,
    vk::ImageLayout f_newLayout
)
{
    vk::AccessFlagBits2 l_srcAccessMask;
    vk::AccessFlagBits2 l_dstAccessMask;

    vk::PipelineStageFlags2 l_sourceStage;
    vk::PipelineStageFlags2 l_destinationStage;

    if (f_oldLayout == vk::ImageLayout::eUndefined
        && f_newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        l_srcAccessMask = {};
        l_dstAccessMask = vk::AccessFlagBits2::eTransferWrite;

        l_sourceStage      = vk::PipelineStageFlagBits2::eTopOfPipe;
        l_destinationStage = vk::PipelineStageFlagBits2::eTransfer;
    }
    else if (f_oldLayout == vk::ImageLayout::eTransferDstOptimal
             && f_newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        l_srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        l_dstAccessMask = vk::AccessFlagBits2::eShaderRead;

        l_sourceStage      = vk::PipelineStageFlagBits2::eTransfer;
        l_destinationStage = vk::PipelineStageFlagBits2::eFragmentShader;
    }
    else {
        throw std::runtime_error(
            "ImageVulkan::transitionImageLayout(...) unsupported layout transition!"
        );
    }

    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->transitionImageLayout(
        this,
        f_oldLayout,
        f_newLayout,
        l_srcAccessMask,
        l_dstAccessMask,
        l_sourceStage,
        l_destinationStage
    );
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();
}

void ImageVulkan::copyBufferToImage(
    utils::VmaBuffer& f_buffer,
    uint32_t          f_width,
    uint32_t          f_height
)
{
    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->copyBuffer(f_buffer, m_image, f_width, f_height);
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();
}

}   // namespace image
}   // namespace renderer
