#ifndef IMAGE_VULKAN_HPP_INCLUDED
#define IMAGE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>

#include "renderer/image/inc/image.hpp"

namespace renderer {
namespace image {

class ImageVulkan : public Image {
public:
    static vk::Format convertToVkFormat(const ImageFormat f_format);
    static vk::ColorSpaceKHR convertToVkColorSpace(const ColorSpace f_colorSpace);

private:
};
}   // namespace image
}   // namespace renderer

#endif
