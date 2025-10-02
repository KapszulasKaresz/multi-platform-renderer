#ifndef RENDER_TARGET_VULKAN_HPP_INCLUDED
#define RENDER_TARGET_VULKAN_HPP_INCLUDED

#include "renderer/render_target/inc/render_target.hpp"

namespace renderer {
namespace image {
class ImageVulkan;
}   // namespace image

namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace render_target {

class RenderTargetVulkan : public RenderTarget {
public:
    RenderTargetVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    RenderTargetVulkan&           create() override final;
    std::shared_ptr<image::Image> getImage() override final;
    std::shared_ptr<image::Image> getDepthImage() override final;
    glm::ivec2                    getSize() const override final;

protected:
    void createColorResources();
    void createDepthResources();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    std::shared_ptr<image::ImageVulkan> m_depthImage{ nullptr };
    std::shared_ptr<image::ImageVulkan> m_colorImage{ nullptr };
};

}   // namespace render_target
}   // namespace renderer

#endif
