#include "renderer/image/inc/image_vulkan.hpp"

namespace renderer {
namespace image {
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

vk::ColorSpaceKHR ImageVulkan::convertToVkColorSpace(const ColorSpace f_colorSpace) {
    switch (f_colorSpace)
    {
        case COLOR_SPACE_LINEAR:
        {
            return vk::ColorSpaceKHR::eExtendedSrgbLinearEXT;
        }

        case COLOR_SPACE_SRGB_NON_LINEAR:
        {
            return vk::ColorSpaceKHR::eSrgbNonlinear;
        }

        case COLOR_SPACE_MAX:
        default:
        {
            return vk::ColorSpaceKHR::eSrgbNonlinear; // safe fallback
        }
    }
}
}   // namespace image
}   // namespace renderer
