#ifndef RENDERING_DEVICE_DX_HPP_INCLUDED
#define RENDERING_DEVICE_DX_HPP_INCLUDED


#include <d3d12.h>
#include <D3D12MemAlloc.h>
#include <dxgi1_6.h>
#include <imgui_impl_dx12.h>
#include <wrl.h>

#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/utils/inc/descriptor_heap_manager_dx.hpp"

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

    rendering_api::RenderingApiDX*                 getParentApi();
    ID3D12CommandQueue*                            getCommandQueue();
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> getCommandAllocator(int f_index = -1);
    Microsoft::WRL::ComPtr<ID3D12Device>           getDevice();
    D3D12MA::Allocator*                            getMemoryAllocator();
    utils::DescriptorHeapManagerDX*                getCommonDescriptorHeapManager();
    utils::DescriptorHeapManagerDX*                getCommonSamplerHeapManager();

    void executeCommandList(ID3D12GraphicsCommandList* f_commandList);

    ~RenderingDeviceDX();

private:
    void createAdapter();
    void createDevice();
    void createAllocator();
    void createCommandQueue();
    void createSyncObjects();
    void createRenderTargetWindow();
    void createDescriptorHeapManager();
    void initImGui();

    static void imguiAllocation(
        ImGui_ImplDX12_InitInfo*,
        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle
    );

    static void imguiFree(
        ImGui_ImplDX12_InitInfo*,
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle
    );

    rendering_api::RenderingApiDX* m_parentApi{ nullptr };

    Microsoft::WRL::ComPtr<IDXGIAdapter4>      m_adapter{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12Device>       m_device{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12DebugDevice>  m_debugDevice{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_allocator{ nullptr };

    window::Window*                                      m_window{ nullptr };
    std::shared_ptr<render_target::RenderTargetWindowDX> m_renderTargetWindow{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>     m_commandQueue{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[maxFramesInFlight];

    std::shared_ptr<command_buffer::CommandBufferDX> m_renderingCommandBuffer{ nullptr };

    std::shared_ptr<utils::DescriptorHeapManagerDX> m_commonDescriptorHeap{ nullptr };
    std::shared_ptr<utils::DescriptorHeapManagerDX> m_commonSamplerHeap{ nullptr };

    // Sync objects
    UINT                                m_frameIndex{ 0 };
    HANDLE                              m_fenceEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence{ nullptr };
    UINT64                              m_fenceValues[maxFramesInFlight];
};

}   // namespace rendering_device
}   // namespace renderer

#endif
