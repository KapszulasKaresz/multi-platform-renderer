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

    UINT getHeapOffset();
    UINT getSamplerOffset();

    static D3D12_FILTER convertToD3D12Filter(Filter f_filter, MipMapMode f_mipmapMode);
    static D3D12_TEXTURE_ADDRESS_MODE convertToD3D12AddressMode(AddressMode f_addressMode);

    ~TextureDX();

private:
    void createShaderResourceView();
    void createSampler();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    D3D12_SAMPLER_DESC m_samplerDesc{};

    UINT m_heapPosition{ 0 };
    UINT m_heapPositionSampler{ 0 };
};
}   // namespace texture
}   // namespace renderer

#endif
