#include "renderer/rendering_api/inc/rendering_api_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace rendering_api {
RenderingApiDX::RenderingApiDX()
{
    m_apiType = RENDERING_API_DIRECTX12;
}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::getMainRenderingDevice()
{
    if (!m_mainRenderingDevice) {
        auto l_renderingDevice = createRenderingDevice();
        auto l_renderingDeviceDXRaw =
            dynamic_cast<rendering_device::RenderingDeviceDX*>(l_renderingDevice.get());
        if (l_renderingDeviceDXRaw == nullptr) {
            throw std::
                runtime_error(
                    "RenderingApDX::getMainRenderingDevice() somehow "
                    "renderingdevice isn't a dx rendering device"
                );
        }
        l_renderingDeviceDXRaw->create();

        m_mainRenderingDevice = l_renderingDevice;
    }

    return m_mainRenderingDevice;
}

void RenderingApiDX::createMainRenderingDeviceWindow(window::Window* f_window)
{
    if (!m_mainRenderingDevice) {
        auto l_renderingDevice = createRenderingDevice();
        auto l_renderingDeviceDXRaw =
            dynamic_cast<rendering_device::RenderingDeviceDX*>(l_renderingDevice.get());
        if (l_renderingDeviceDXRaw == nullptr) {
            throw std::
                runtime_error(
                    "RenderingApDX::getMainRenderingDevice() somehow "
                    "renderingdevice isn't a dx rendering device"
                );
        }
        l_renderingDeviceDXRaw->setWindow(f_window).create();

        m_mainRenderingDevice = l_renderingDevice;
    }
    else {
        throw std::
            runtime_error(
                "RenderingApiDX::createMainRenderingDeviceWindow() main "
                "renderingdevice already created"
            );
    }
}

RenderingApiDX& RenderingApiDX::enableDebuging(bool f_enable)
{
    m_debugEnabled = f_enable;
    return *this;
}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::createRenderingDevice()
{
    return std::make_shared<rendering_device::RenderingDeviceDX>(this);
}

RenderingApiDX& RenderingApiDX::create()
{
    createFactory();

    m_valid = true;
    return *this;
}

Microsoft::WRL::ComPtr<IDXGIFactory7> RenderingApiDX::getFactory()
{
    return m_dxgiFactory;
}

void RenderingApiDX::createFactory()
{
    if (m_debugEnabled) {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)))) {
            m_debugController->EnableDebugLayer();
        }
        else {
            throw std::runtime_error(
                "RenderingApiDX::createFactory() failed to enbale debuging"
            );
        }
    }

    UINT l_flags = 0;
    if (m_debugEnabled) {
        l_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }


    if (FAILED(CreateDXGIFactory2(l_flags, IID_PPV_ARGS(&m_dxgiFactory)))) {
        throw std::runtime_error(
            "RenderingApiDirectX12::createFactory() Failed to create DXGI Factory"
        );
    }
}
}   // namespace rendering_api
}   // namespace renderer
