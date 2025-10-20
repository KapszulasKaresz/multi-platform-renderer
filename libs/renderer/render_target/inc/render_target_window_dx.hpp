#ifndef RENDER_TARGET_WINDOW_DX_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_DX_HPP_INCLUDED

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/render_target/inc/render_target_window.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace render_target {

class RenderTargetWindowDX : public RenderTargetWindow {
public:
    RenderTargetWindowDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    RenderTargetWindowDX&         setWindow(window::Window* f_window) override final;
    std::shared_ptr<image::Image> getImage() override final;
    std::shared_ptr<image::Image> getDepthImage() override final;
    glm::ivec2                    getSize() const override final;

    RenderTargetWindowDX& create();

private:
    void createSwapCahin();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain{ nullptr };

    static const UINT s_backbufferCount = 2;
    UINT              m_currentBuffer{ 0 };
};

}   // namespace render_target
}   // namespace renderer

#endif
