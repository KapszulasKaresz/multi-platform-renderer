#ifndef TEXTURE_VULKAN_HPP_INCLUDED
#define TEXTURE_VULKAN_HPP_INCLUDED

#include "renderer/texture/inc/texture.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace texture {
class TextureVulkan : public Texture {
public:
    TextureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    TextureVulkan& create() override final;

private:
    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };
};
}   // namespace texture
}   // namespace renderer

#endif
