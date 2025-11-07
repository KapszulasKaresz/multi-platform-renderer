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
    return m_size;
}

RenderTargetWindowDX& RenderTargetWindowDX::create()
{
    createSwapCahin();
    createDescriptorHeap();
    createRenderTargets();
    m_valid = true;
    return *this;
}

ID3D12Resource* RenderTargetWindowDX::getRenderTarget()
{
    return m_renderTargets[m_parentDevice->getCurrentFrame()].Get();
}

ID3D12DescriptorHeap* RenderTargetWindowDX::getDescriptorHeap()
{
    return m_rtvHeap.Get();
}

UINT RenderTargetWindowDX::getDescriptorSize()
{
    return m_rtvDescriptorSize;
}

IDXGISwapChain3* RenderTargetWindowDX::getSwapchain()
{
    return m_swapChain.Get();
}

void RenderTargetWindowDX::resizeSwapChain()
{
    createSwapCahin();
}

void RenderTargetWindowDX::createSwapCahin()
{
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
    m_size = glm::ivec2(m_window->getWidth(), m_window->getHeight());

    m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();
    m_parentDevice->waitForGPU();
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
