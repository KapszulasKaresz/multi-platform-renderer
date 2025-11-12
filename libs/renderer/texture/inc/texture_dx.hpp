#ifndef TEXTURE_DX_INCLUDED
#define TEXTURE_DX_INCLUDED
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/texture/inc/texture.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace texture {
class TextureDX : public Texture {
public:
    TextureDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    TextureDX& create() override final;

    ID3D12DescriptorHeap* getDescriptorHeap();

private:
    void createShaderResourceView();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};
}   // namespace texture
}   // namespace renderer

#endif
