#include "renderer/render_target/inc/render_target_window_dx.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/rendering_api/inc/rendering_api_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace render_target {
RenderTargetWindowDX::RenderTargetWindowDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : RenderTargetWindow(),
      m_parentDevice(f_parentDevice)
{}

RenderTargetWindowDX& RenderTargetWindowDX::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

std::shared_ptr<image::Image> RenderTargetWindowDX::getImage()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<image::Image> RenderTargetWindowDX::getDepthImage()
{
    throw std::logic_error("Function not yet implemented");
}

glm::ivec2 RenderTargetWindowDX::getSize() const
{
    throw std::logic_error("Function not yet implemented");
}

RenderTargetWindowDX& RenderTargetWindowDX::create()
{
    createSwapCahin();
    createDescriptorHeap();
    createRenderTargets();
    m_valid = true;
    return *this;
}

void RenderTargetWindowDX::createSwapCahin()
{
    D3D12_VIEWPORT l_viewport{ .TopLeftX = 0.0f,
                               .TopLeftY = 0.0f,
                               .Width    = static_cast<float>(m_window->getWidth()),
                               .Height   = static_cast<float>(m_window->getHeight()),
                               .MinDepth = .1f,
                               .MaxDepth = 1000.f };
    D3D12_RECT     l_surfaceSize{ .left   = 0,
                                  .top    = 0,
                                  .right  = static_cast<LONG>(m_window->getWidth()),
                                  .bottom = static_cast<LONG>(m_window->getHeight()) };


    if (m_swapChain != nullptr) {
        m_swapChain->ResizeBuffers(
            s_backbufferCount,
            m_window->getWidth(),
            m_window->getHeight(),
            image::ImageDX::convertToDXFormat(m_format),
            0
        );
    }
    else {
        DXGI_SWAP_CHAIN_DESC1 l_swapchainDesc = {};
        l_swapchainDesc.BufferCount           = s_backbufferCount;
        l_swapchainDesc.Width                 = m_window->getWidth();
        l_swapchainDesc.Height                = m_window->getHeight();
        l_swapchainDesc.Format           = image::ImageDX::convertToDXFormat(m_format);
        l_swapchainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        l_swapchainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        l_swapchainDesc.SampleDesc.Count = 1;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> l_swapChain;
        if (FAILED(m_parentDevice->getParentApi()->getFactory()->CreateSwapChainForHwnd(
                m_parentDevice->getCommandQueue(),
                static_cast<HWND>(m_window->getHwnd()),
                &l_swapchainDesc,
                nullptr,
                nullptr,
                &l_swapChain
            )))
        {
            throw std::runtime_error(
                "RenderTargetWindowDX::createSwapCahin() failed to create swpachain"
            );
        }

        if (FAILED(l_swapChain.As(&m_swapChain))) {
            throw std::runtime_error(
                "RenderTargetWindowDX::createSwapCahin() failed to upcast swpachain"
            );
        }
    }

    m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();
}

void RenderTargetWindowDX::createDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC l_rtvHeapDesc = {};
    l_rtvHeapDesc.NumDescriptors             = rendering_device::maxFramesInFlight;
    l_rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    l_rtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createDescriptorHeap() failed to create descriptor heap"
        );
    }

    m_rtvDescriptorSize = m_parentDevice->getDevice()->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV
    );
}

void RenderTargetWindowDX::createRenderTargets()
{
    D3D12_CPU_DESCRIPTOR_HANDLE l_rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    for (UINT n = 0; n < rendering_device::maxFramesInFlight; n++) {
        if (FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])))) {
            throw std::runtime_error(
                "RenderTargetWindowDX::createRenderTargets() failed to create render "
                "target"
            );
        }
        m_parentDevice->getDevice()->CreateRenderTargetView(
            m_renderTargets[n].Get(), nullptr, l_rtvHandle
        );
        l_rtvHandle.ptr += (1 * m_rtvDescriptorSize);
    }
}
}   // namespace render_target
}   // namespace renderer
