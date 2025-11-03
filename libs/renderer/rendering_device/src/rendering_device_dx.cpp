#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

#include <stdexcept>

#include "renderer/command_buffer/inc/command_buffer_dx.hpp"
#include "renderer/mesh/inc/triangle_mesh_dx.hpp"
#include "renderer/render_target/inc/render_target_window_dx.hpp"
#include "renderer/rendering_api/inc/rendering_api_dx.hpp"

namespace renderer {
namespace rendering_device {

RenderingDeviceDX::RenderingDeviceDX(rendering_api::RenderingApiDX* f_parentApi)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceDX::getRenderTargetWindow()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceDX::getRenderTargetWindow() device needs to be valid"
        );
    }

    if (!m_window) {
        throw std::
            runtime_error(
                "RenderingDeviceDX::getRenderTargetWindow() needs a window "
                "attached to the device"
            );
    }

    if (!m_renderTargetWindow) {
        throw std::
            runtime_error(
                "RenderingDeviceDX::getRenderTargetWindow() no render target window "
                "is available"
            );
    }

    return m_renderTargetWindow;
}

std::shared_ptr<image::Image> RenderingDeviceDX::createImage()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<material::Material> RenderingDeviceDX::createMaterial()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<command_buffer::CommandBuffer> RenderingDeviceDX::createCommandBuffer()
{
    auto l_commandBuffer = std::make_shared<command_buffer::CommandBufferDX>(this);
    l_commandBuffer->setRendering(false).create();
    return l_commandBuffer;
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceDX::getRenderingCommandBuffer()
{
    if (!m_renderingCommandBuffer) {
        m_renderingCommandBuffer = std::make_shared<command_buffer::CommandBufferDX>(this);
        m_renderingCommandBuffer->setRendering(true).create();
    }
    return m_renderingCommandBuffer;
}

std::shared_ptr<mesh::TriangleMesh> RenderingDeviceDX::createTriangleMesh()
{
    return std::make_shared<mesh::TriangleMeshDX>(this);
}

std::shared_ptr<uniform::UniformCollection> RenderingDeviceDX::createUniformCollection()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<texture::Texture> RenderingDeviceDX::createTexture()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<render_target::RenderTarget> RenderingDeviceDX::createRenderTarget()
{
    throw std::logic_error("Function not yet implemented");
}

bool RenderingDeviceDX::preFrame()
{
    throw std::logic_error("Function not yet implemented");
}

void RenderingDeviceDX::postFrame()
{
    throw std::logic_error("Function not yet implemented");
}

void RenderingDeviceDX::finishRendering()
{
    throw std::logic_error("Function not yet implemented");
}

RenderingDeviceDX& RenderingDeviceDX::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderingDeviceDX& RenderingDeviceDX::create()
{
    createAdapter();
    createDevice();
    createAllocator();
    createCommandQueue();
    m_valid = true;
    createRenderTargetWindow();
    return *this;
}

void RenderingDeviceDX::waitForGPU()
{
    if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]))) {
        throw std::runtime_error(
            "RenderingDeviceDX::waitForGPU() Failed to signal command queue"
        );
    }

    if (FAILED(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent)))
    {
        throw std::runtime_error("RenderingDeviceDX::waitForGPU() Failed to set fence");
    }
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    m_fenceValues[m_frameIndex]++;
}

rendering_api::RenderingApiDX* RenderingDeviceDX::getParentApi()
{
    return m_parentApi;
}

ID3D12CommandQueue* RenderingDeviceDX::getCommandQueue()
{
    return m_commandQueue.Get();
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RenderingDeviceDX::getCommandAllocator()
{
    return m_commandAllocators[m_currentFrame];
}

Microsoft::WRL::ComPtr<ID3D12Device> RenderingDeviceDX::getDevice()
{
    return m_device;
}

D3D12MA::Allocator* RenderingDeviceDX::getMemoryAllocator()
{
    return m_allocator.Get();
}

RenderingDeviceDX::~RenderingDeviceDX() {}

void RenderingDeviceDX::createAdapter()
{
    IDXGIAdapter4* l_adapter4 = nullptr;
    for (UINT l_index = 0;
         m_parentApi->getFactory()->EnumAdapterByGpuPreference(
             l_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&l_adapter4)
         )
         != DXGI_ERROR_NOT_FOUND;
         ++l_index)
    {
        DXGI_ADAPTER_DESC3 l_desc;
        l_adapter4->GetDesc3(&l_desc);

        if (l_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            l_adapter4->Release();
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(
                l_adapter4, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr
            )))
        {
            m_adapter = l_adapter4;
            break;
        }

        l_adapter4->Release();
    }
}

void RenderingDeviceDX::createDevice()
{
    if (FAILED(D3D12CreateDevice(
            m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createDevice() Failed to create D3D12 device!"
        );
    }

    if (m_parentApi->isDebugEnabled()) {
        if (FAILED(m_device.As(&m_debugDevice))) {
            throw std::runtime_error(
                "RenderingDeviceDX::createDevice() Failed to create D3D12 debug device!"
            );
        }
    }
}

void RenderingDeviceDX::createAllocator()
{
    D3D12MA::ALLOCATOR_DESC l_allocatorDesc = { .Flags =
                                                    D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS,
                                                .pDevice  = m_device.Get(),
                                                .pAdapter = m_adapter.Get() };
    if (FAILED(D3D12MA::CreateAllocator(&l_allocatorDesc, &m_allocator))) {
        throw std::runtime_error(
            "RenderingDeviceDX::createAllocator() failed to create DMA allocator"
        );
    }
}

void RenderingDeviceDX::createCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC l_queueDesc{ .Type  = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                          .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE };
    if (FAILED(m_device->CreateCommandQueue(&l_queueDesc, IID_PPV_ARGS(&m_commandQueue))))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createCommandQueue() Failed to create command queue!"
        );
    }

    for (UINT n = 0; n < maxFramesInFlight; n++) {
        if (FAILED(m_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])
            )))
        {
            throw std::runtime_error(
                "RenderingDeviceDX::createCommandQueue() Failed to create command "
                "allocator!"
            );
        }
    }
}

void RenderingDeviceDX::createSyncObjects()
{
    if (FAILED(m_device->CreateFence(
            m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createSyncObjects() Failed to create fence!"
        );
    }
    m_fenceValues[m_frameIndex]++;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
        if (FAILED((HRESULT_FROM_WIN32(GetLastError())))) {
            throw std::runtime_error(
                "RenderingDeviceDX::createSyncObjects() Failed to create fence event!"
            );
        }
    }
}

void RenderingDeviceDX::createRenderTargetWindow()
{
    if (!m_window) {
        return;
    }
    m_renderTargetWindow = std::make_shared<render_target::RenderTargetWindowDX>(this);
    m_renderTargetWindow->setWindow(m_window)
        .setFormat(image::ImageFormat::IMAGE_FORMAT_BGRA8)
        .setColorSpace(image::ColorSpace::COLOR_SPACE_SRGB_NON_LINEAR)
        .setDepthBuffer(true)
        .create();
}
}   // namespace rendering_device
}   // namespace renderer
