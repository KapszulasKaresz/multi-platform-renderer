#include "renderer/image/inc/image_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

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

    createImageView();
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

void ImageVulkan::createImageView()
{
    vk::ImageViewCreateInfo l_imageViewCreateInfo{
        .image            = m_image,
        .viewType         = vk::ImageViewType::e2D,
        .format           = convertToVkFormat(m_format),
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    };

    m_imageView =
        vk::raii::ImageView(m_parentDevice->getLogicalDevice(), l_imageViewCreateInfo);
}
}   // namespace image
}   // namespace renderer
