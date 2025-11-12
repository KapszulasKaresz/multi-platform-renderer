#ifndef IMAGE_DX_HPP_INCLUDED
#define IMAGE_DX_HPP_INCLUDED

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/image/inc/image.hpp"

#include "D3D12MemAlloc.h"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace image {

class ImageDX : public Image {
public:
    ImageDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    ImageDX& create() override final;
    ImageDX& createFromFile(std::string_view f_path) override final;
    ImageDX& createEmptyImage() override final;

    ImageDX& setFormat(image::ImageFormat f_format) override final;
    ImageDX& setColorSpace(image::ColorSpace f_colorSpace) override final;
    ImageDX& setSize(const glm::ivec2& f_size) override final;

    ID3D12Resource* getResource();

    static DXGI_FORMAT convertToDXFormat(const ImageFormat f_format);

private:
    inline UINT64 GetRequiredIntermediateSize(
        _In_ ID3D12Resource*                       f_pDestinationResource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT f_FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - f_FirstSubresource) UINT f_NumSubresources
    );

    rendering_device::RenderingDeviceDX* m_parentDevice;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_image{ nullptr };
};
}   // namespace image
}   // namespace renderer

#endif
