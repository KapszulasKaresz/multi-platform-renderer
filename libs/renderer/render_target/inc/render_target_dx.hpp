#ifndef RENDER_TARGET_DX_HPP_INCLUDED
#define RENDER_TARGET_DX_HPP_INCLUDED

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

class RenderTargetDX : public RenderTarget {
public:
    RenderTargetDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    std::shared_ptr<image::Image> getImage() override final;
    std::shared_ptr<image::Image> getDepthImage() override final;
    glm::ivec2                    getSize() const override final;

    RenderTargetDX&       create() override;
    ID3D12DescriptorHeap* getMSAADescriptorHeap();
    ID3D12DescriptorHeap* getDepthDescriptorHeap();

    bool isMSAA();

private:
    void createDescriptorHeap();
    void createDepthResource();
    void createColorResource();
    void createRenderTargets();
    void validateMSAARequirements();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_msaaRtvHeap{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap{ nullptr };

    std::shared_ptr<image::ImageDX> m_depthImage{ nullptr };
    std::shared_ptr<image::ImageDX> m_colorImage{ nullptr };

    DXGI_SAMPLE_DESC m_MSAASampleDesc{};
};

}   // namespace render_target
}   // namespace renderer

#endif
