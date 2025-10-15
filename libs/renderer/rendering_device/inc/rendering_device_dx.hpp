#ifndef RENDERING_DEVICE_DX_HPP_INCLUDED
#define RENDERING_DEVICE_DX_HPP_INCLUDED


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace command_buffer {
class CommandBufferVulkan;
}   // namespace command_buffer

namespace image {
class ImageVulkan;
}   // namespace image

namespace rendering_api {
class RenderingApiDX;
}   // namespace rendering_api

namespace render_target {
class RenderTarget;
class RenderTargetWindowVulkan;
}   // namespace render_target

namespace rendering_device {

class RenderingDeviceDX : public RenderingDevice {
public:
    RenderingDeviceDX(rendering_api::RenderingApiDX* f_parentApi);

    std::shared_ptr<render_target::RenderTargetWindow>
                                                   getRenderTargetWindow() override final;
    std::shared_ptr<image::Image>                  createImage() override final;
    std::shared_ptr<material::Material>            createMaterial() override final;
    std::shared_ptr<command_buffer::CommandBuffer> createCommandBuffer() override final;
    std::shared_ptr<command_buffer::CommandBuffer>
                                        getRenderingCommandBuffer() override final;
    std::shared_ptr<mesh::TriangleMesh> createTriangleMesh() override final;
    std::shared_ptr<uniform::UniformCollection>  createUniformCollection() override final;
    std::shared_ptr<texture::Texture>            createTexture() override final;
    std::shared_ptr<render_target::RenderTarget> createRenderTarget() override final;

    bool preFrame() override final;
    void postFrame() override final;
    void finishRendering() override final;

    RenderingDeviceDX& setWindow(window::Window* f_window) override final;
    RenderingDeviceDX& create() override final;


    ~RenderingDeviceDX();

private:
    void createAdapter();

    rendering_api::RenderingApiDX* m_parentApi{ nullptr };

    window::Window* m_window{ nullptr };

    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter{ nullptr };
};

}   // namespace rendering_device
}   // namespace renderer

#endif
