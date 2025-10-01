#ifndef IMAGE_VULKAN_HPP_INCLUDED
#define IMAGE_VULKAN_HPP_INCLUDED

#include <optional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/image/inc/image.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"
#include "renderer/utils/inc/vulkan_image_utils.hpp"

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
    ImageVulkan& createFromFile(std::string_view f_path) override final;
    ImageVulkan& createEmptyImage() override final;

    ImageVulkan& setFormat(image::ImageFormat f_format) override final;
    ImageVulkan& setColorSpace(image::ColorSpace f_colorSpace) override final;
    ImageVulkan& setSize(const glm::ivec2& f_size) override final;
    ImageVulkan& setShaderStageDestination(
        vk::PipelineStageFlagBits2 f_shaderStageDestination
    );
    vk::PipelineStageFlagBits2 getShaderStageDestination() const;

    ImageVulkan& setSampleCount(uint32_t f_samples);
    ImageVulkan& setUsage(vk::ImageUsageFlags f_usage);

    static vk::Format        convertToVkFormat(const ImageFormat f_format);
    static vk::ColorSpaceKHR convertToVkColorSpace(const ColorSpace f_colorSpace);

    vk::Image     getImage();
    vk::ImageView getImageView();

    static vk::Format findDepthFormat(vk::PhysicalDevice f_physicalDevice);

private:
    vk::raii::ImageView
        createImageView(vk::Image& f_image, vk::ImageAspectFlags f_aspectFlags);

    void transitionImageLayout(vk::ImageLayout f_oldLayout, vk::ImageLayout f_newLayout);
    void copyBufferToImage(utils::VmaBuffer& f_buffer, uint32_t f_width, uint32_t f_height);

    void createMipmaps();

    static vk::Format findSupportedFormat(
        const std::vector<vk::Format>& f_candidates,
        vk::ImageTiling                f_tiling,
        vk::FormatFeatureFlags         f_features,
        vk::PhysicalDevice             f_physicalDevice
    );

    rendering_device::RenderingDeviceVulkan* m_parentDevice;

    utils::VmaImage     m_image{};
    vk::raii::ImageView m_imageView{ nullptr };

    vk::PipelineStageFlagBits2 m_shaderStageDestination{
        vk::PipelineStageFlagBits2::eFragmentShader
    };

    uint32_t            m_samples{ 1 };
    vk::ImageUsageFlags m_usage{ vk::ImageUsageFlagBits::eSampled };

    vk::Format               m_vkFormat{ vk::Format::eUndefined };
    std::optional<vk::Image> m_swapchainImage{};
};
}   // namespace image
}   // namespace renderer

#endif
