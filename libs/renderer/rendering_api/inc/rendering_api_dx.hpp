#ifndef RENDERING_API_DX_HPP_INCLUDED
#define RENDERING_API_DX_HPP_INCLUDED

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {

class RenderingApiDX : public RenderingApi {
public:
    RenderingApiDX();

    rendering_device::RenderingDevice* getMainRenderingDevice() override final;

    void createMainRenderingDeviceWindow(window::Window* f_window) override final;

    RenderingApiDX& enableDebuging(bool f_enable = true);

    std::shared_ptr<rendering_device::RenderingDevice>
        createRenderingDevice() override final;

    RenderingApiDX& create();

    Microsoft::WRL::ComPtr<IDXGIFactory7> getFactory();
    bool                                  isDebugEnabled() const;

private:
    void createFactory();

    bool m_debugEnabled{ false };

    Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12Debug>   m_debugController{ nullptr };
};
}   // namespace rendering_api
}   // namespace renderer

#endif
