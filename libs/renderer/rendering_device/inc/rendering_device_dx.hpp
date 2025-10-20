#ifndef RENDERING_DEVICE_DX_HPP_INCLUDED
#define RENDERING_DEVICE_DX_HPP_INCLUDED


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace command_buffer {
class CommandBufferDX;
}   // namespace command_buffer

namespace image {
class ImageDX;
}   // namespace image

namespace rendering_api {
class RenderingApiDX;
}   // namespace rendering_api

namespace render_target {
class RenderTarget;
class RenderTargetWindowDX;
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

    void waitForGPU();

    rendering_api::RenderingApiDX* getParentApi();
    ID3D12CommandQueue*            getCommandQueue();

    ~RenderingDeviceDX();

private:
    void createAdapter();
    void createDevice();
    void createCommandQueue();
    void createSyncObjects();
    void createRenderTargetWindow();

    rendering_api::RenderingApiDX* m_parentApi{ nullptr };

    window::Window*                                      m_window{ nullptr };
    std::shared_ptr<render_target::RenderTargetWindowDX> m_renderTargetWindow{ nullptr };

    Microsoft::WRL::ComPtr<IDXGIAdapter4>     m_adapter{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12Device>      m_device{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12DebugDevice> m_debugDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>     m_commandQueue{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator{ nullptr };

    // Sync objects
    UINT                                m_frameIndex{ 0 };
    HANDLE                              m_fenceEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence{ nullptr };
    UINT64                              m_fenceValues[maxFramesInFlight];
};

}   // namespace rendering_device
}   // namespace renderer

#endif
