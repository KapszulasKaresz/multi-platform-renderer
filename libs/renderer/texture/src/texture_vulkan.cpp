#include "renderer/texture/inc/texture_vulkan.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace texture {
TextureVulkan::TextureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice)
    : m_parentDevice(f_parentDevice)
{}

TextureVulkan& TextureVulkan::create()
{
    createTextureSampler();
    m_valid = true;
    return *this;
}

vk::Sampler TextureVulkan::getSampler()
{
    return m_sampler;
}

vk::ImageView TextureVulkan::getImageView()
{
    image::ImageVulkan* l_rawVulkanImage =
        dynamic_cast<image::ImageVulkan*>(m_image.get());

    if (l_rawVulkanImage == nullptr) {
        throw std::runtime_error(
            "TextureVulkan::getImageView() image isn't a vulkan image"
        );
    }

    return l_rawVulkanImage->getImageView();
}

vk::PipelineStageFlagBits2 TextureVulkan::getShaderStageDestination() const
{
    image::ImageVulkan* l_rawVulkanImage =
        dynamic_cast<image::ImageVulkan*>(m_image.get());

    if (l_rawVulkanImage == nullptr) {
        throw std::runtime_error(
            "TextureVulkan::getShaderStageDestination() image isn't a vulkan image"
        );
    }

    return l_rawVulkanImage->getShaderStageDestination();
}

void TextureVulkan::createTextureSampler()
{
    vk::PhysicalDeviceProperties l_properties =
        m_parentDevice->getPhysicalDevice().getProperties();
    vk::SamplerCreateInfo l_samplerInfo{ .magFilter    = vk::Filter::eLinear,
                                         .minFilter    = vk::Filter::eLinear,
                                         .mipmapMode   = vk::SamplerMipmapMode::eLinear,
                                         .addressModeU = vk::SamplerAddressMode::eRepeat,
                                         .addressModeV = vk::SamplerAddressMode::eRepeat,
                                         .addressModeW = vk::SamplerAddressMode::eRepeat,
                                         .mipLodBias   = 0.0f,
                                         .anisotropyEnable = vk::True,
                                         .maxAnisotropy =
                                             l_properties.limits.maxSamplerAnisotropy,
                                         .compareEnable = vk::False,
                                         .compareOp     = vk::CompareOp::eAlways };
    m_sampler = vk::raii::Sampler(m_parentDevice->getLogicalDevice(), l_samplerInfo);
}

}   // namespace texture
}   // namespace renderer
