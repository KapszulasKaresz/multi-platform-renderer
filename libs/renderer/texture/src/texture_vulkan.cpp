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

vk::Filter TextureVulkan::convertToVkFilter(Filter f_filter)
{
    switch (f_filter) {
        case FILTER_LINEAR: {
            return vk::Filter::eLinear;
        }
        case FILTER_NEAREST: {
            return vk::Filter::eNearest;
        }
        case FILTER_CUBIC: {
            return vk::Filter::eCubicIMG;
        }
        case FILTER_MAX:
        default:         {
            return vk::Filter::eLinear;
        }
    }
}

vk::SamplerMipmapMode TextureVulkan::converToVkMipMapMode(MipMapMode f_mode)
{
    switch (f_mode) {
        case MIPMAP_MODE_LINEAR: {
            return vk::SamplerMipmapMode::eLinear;
        }
        case MIPMAP_MODE_NEAREST: {
            return vk::SamplerMipmapMode::eNearest;
        }
        case MIPMAP_MODE_MAX:
        default:              {
            return vk::SamplerMipmapMode::eLinear;
        }
    }
}

vk::SamplerAddressMode TextureVulkan::convertToVkAddressMode(AddressMode f_addressMode)
{
    switch (f_addressMode) {
        case ADDRESS_MODE_REPEAT: {
            return vk::SamplerAddressMode::eRepeat;
        }
        case ADDRESS_MODE_MIRRORED_REPEAT: {
            return vk::SamplerAddressMode::eMirroredRepeat;
        }
        case ADDRESS_MODE_CLAMP_TO_EDGE: {
            return vk::SamplerAddressMode::eClampToEdge;
        }
        case ADDRESS_MODE_CLAMP_TO_BORDER: {
            return vk::SamplerAddressMode::eClampToBorder;
        }
        case ADDRESS_MODE_MIRRORED_CLAMP_TO_EDGE: {
            return vk::SamplerAddressMode::eMirrorClampToEdge;
        }
        case ADDRESS_MODE_MAX:
        default:               {
            return vk::SamplerAddressMode::eRepeat;
        }
    }
}

void TextureVulkan::createTextureSampler()
{
    vk::PhysicalDeviceProperties l_properties =
        m_parentDevice->getPhysicalDevice().getProperties();
    vk::SamplerCreateInfo l_samplerInfo{
        .magFilter        = convertToVkFilter(m_magFilter),
        .minFilter        = convertToVkFilter(m_minFilter),
        .mipmapMode       = converToVkMipMapMode(m_mipMapMode),
        .addressModeU     = convertToVkAddressMode(m_addressModeU),
        .addressModeV     = convertToVkAddressMode(m_addressModeV),
        .addressModeW     = convertToVkAddressMode(m_addressModeW),
        .mipLodBias       = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy    = l_properties.limits.maxSamplerAnisotropy,
        .compareEnable    = vk::False,
        .compareOp        = vk::CompareOp::eAlways
    };
    m_sampler = vk::raii::Sampler(m_parentDevice->getLogicalDevice(), l_samplerInfo);
}

}   // namespace texture
}   // namespace renderer
