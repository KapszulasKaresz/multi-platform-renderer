#include "renderer/rendering_api/inc/rendering_api_dx.hpp"

#include <stdexcept>

namespace renderer {
namespace rendering_api {
RenderingApiDX::RenderingApiDX()
{
    m_apiType = RENDERING_API_DIRECTX12;
}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::getMainRenderingDevice()
{
    throw std::logic_error("Function not yet implemented");
    return nullptr;
}

void RenderingApiDX::createMainRenderingDeviceWindow(window::Window* f_window) {}

RenderingApiDX& RenderingApiDX::enableDebuging(bool f_enable)
{
    m_debugEnabled = f_enable;
    return *this;
}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::createRenderingDevice()
{
    throw std::logic_error("Function not yet implemented");
    return nullptr;
}

RenderingApiDX& RenderingApiDX::create()
{
    createFactory();

    m_valid = true;
    return *this;
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
