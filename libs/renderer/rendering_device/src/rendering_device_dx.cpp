#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

#include <stdexcept>

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
    throw std::logic_error("Function not yet implemented");
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
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceDX::getRenderingCommandBuffer()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<mesh::TriangleMesh> RenderingDeviceDX::createTriangleMesh()
{
    throw std::logic_error("Function not yet implemented");
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
    createCommandQueue();
    m_valid = true;
    return *this;
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

    if (FAILED(m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createCommandQueue() Failed to create command allocator!"
        );
    }
}

void RenderingDeviceDX::createSyncObjects() {}
}   // namespace rendering_device
}   // namespace renderer
