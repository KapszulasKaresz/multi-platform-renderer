#ifndef RENDER_TARGET_WINDOW_DX_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_DX_HPP_INCLUDED

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"

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
    ID3D12Resource*       getRenderTarget();
    ID3D12DescriptorHeap* getDescriptorHeap();
    ID3D12DescriptorHeap* getMSAADescriptorHeap();
    ID3D12DescriptorHeap* getDepthDescriptorHeap();
    UINT                  getDescriptorSize();
    IDXGISwapChain3*      getSwapchain();

    void resizeSwapChain();
    bool isMSAA();

private:
    void createSwapCahin();
    void createDescriptorHeap();
    void createRenderTargets();
    void createDepthResource();
    void createColorResource();
    void validateMSAARequirements();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap{ nullptr };
    UINT                                         m_rtvDescriptorSize;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12Resource>
        m_renderTargets[rendering_device::maxFramesInFlight];

    static const UINT s_backbufferCount = 2;
    UINT              m_currentBuffer{ 0 };

    std::shared_ptr<image::ImageDX> m_depthImage{ nullptr };
    std::shared_ptr<image::ImageDX> m_colorImage{ nullptr };

    DXGI_SAMPLE_DESC m_MSAASampleDesc{};
};

}   // namespace render_target
}   // namespace renderer

#endif
