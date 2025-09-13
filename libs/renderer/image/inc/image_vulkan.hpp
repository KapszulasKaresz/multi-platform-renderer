#ifndef IMAGE_VULKAN_HPP_INCLUDED
#define IMAGE_VULKAN_HPP_INCLUDED

#include <optional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/image/inc/image.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace image {

class ImageVulkan : public Image {
public:
    ImageVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    ImageVulkan& create() override final;
    ImageVulkan& createFromSwapChainImage(vk::Image f_swapChainImage);

    ImageVulkan& setFormat(image::ImageFormat f_format) override final;
    ImageVulkan& setColorSpace(image::ColorSpace f_colorSpace) override final;

    static vk::Format        convertToVkFormat(const ImageFormat f_format);
    static vk::ColorSpaceKHR convertToVkColorSpace(const ColorSpace f_colorSpace);

    vk::Image     getImage();
    vk::ImageView getImageView();

private:
    vk::raii::ImageView
        createImageView(vk::Image& f_image, vk::ImageAspectFlags f_aspectFlags);

    rendering_device::RenderingDeviceVulkan* m_parentDevice;

    vk::raii::Image     m_image{ nullptr };
    vk::raii::ImageView m_imageView{ nullptr };

    std::optional<vk::Image> m_swapchainImage{};
};
}   // namespace image
}   // namespace renderer

#endif
