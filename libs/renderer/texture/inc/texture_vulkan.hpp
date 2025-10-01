#ifndef TEXTURE_VULKAN_HPP_INCLUDED
#define TEXTURE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan_raii.hpp>

#include "renderer/texture/inc/texture.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace texture {
class TextureVulkan : public Texture {
public:
    TextureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    TextureVulkan&             create() override final;
    vk::Sampler                getSampler();
    vk::ImageView              getImageView();
    vk::PipelineStageFlagBits2 getShaderStageDestination() const;

    static vk::Filter             convertToVkFilter(Filter f_filter);
    static vk::SamplerMipmapMode  converToVkMipMapMode(MipMapMode f_mode);
    static vk::SamplerAddressMode convertToVkAddressMode(AddressMode f_addressMode);

private:
    void createTextureSampler();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::Sampler m_sampler{ nullptr };
};
}   // namespace texture
}   // namespace renderer

#endif
