#include "renderer/texture/inc/texture_dx.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace texture {
TextureDX::TextureDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : Texture(),
      m_parentDevice(f_parentDevice)
{}

TextureDX& TextureDX::create()
{
    createShaderResourceView();
    createSampler();
    m_valid = true;
    return *this;
}

UINT TextureDX::getHeapOffset()
{
    return m_heapPosition;
}

UINT TextureDX::getSamplerOffset()
{
    return m_heapPositionSampler;
}

D3D12_FILTER TextureDX::convertToD3D12Filter(Filter f_filter, MipMapMode f_mipmapMode)
{
    switch (f_filter) {
        case FILTER_NEAREST:
            return (f_mipmapMode == MIPMAP_MODE_NEAREST)
                     ? D3D12_FILTER_MIN_MAG_MIP_POINT
                     : D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;

        case FILTER_LINEAR:
            return (f_mipmapMode == MIPMAP_MODE_NEAREST)
                     ? D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT
                     : D3D12_FILTER_MIN_MAG_MIP_LINEAR;

        case FILTER_CUBIC:
            // DX12 doesn’t have a direct “cubic” filter.
            // The best approximation is anisotropic filtering.
            return D3D12_FILTER_ANISOTROPIC;

        case FILTER_MAX:
        default:         return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    }
}

D3D12_TEXTURE_ADDRESS_MODE TextureDX::convertToD3D12AddressMode(AddressMode f_addressMode)
{
    switch (f_addressMode) {
        case ADDRESS_MODE_REPEAT:          return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        case ADDRESS_MODE_MIRRORED_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;

        case ADDRESS_MODE_CLAMP_TO_EDGE:   return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        case ADDRESS_MODE_CLAMP_TO_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;

        case ADDRESS_MODE_MIRRORED_CLAMP_TO_EDGE:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

        case ADDRESS_MODE_MAX:
        default:               return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

void TextureDX::createShaderResourceView()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};
    l_srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_srvDesc.Format        = image::ImageDX::convertToDXFormat(m_image->getFormat());
    l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    l_srvDesc.Texture2D.MostDetailedMip     = 0;
    l_srvDesc.Texture2D.MipLevels           = m_image->getMipLevels();
    l_srvDesc.Texture2D.PlaneSlice          = 0;
    l_srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    image::ImageDX* l_imageDX = dynamic_cast<image::ImageDX*>(m_image.get());

    if (!l_imageDX) {
        throw std::runtime_error(
            "TextureDX::createShaderResourceView() image wasn't correct"
        );
    }

    m_heapPosition = m_parentDevice->getCommonDescriptorHeapManager()->addSRV(
        l_imageDX->getResource(), l_srvDesc
    );
}

void TextureDX::createSampler()
{
    m_samplerDesc.Filter         = convertToD3D12Filter(m_minFilter, m_mipMapMode);
    m_samplerDesc.AddressU       = convertToD3D12AddressMode(m_addressModeU);
    m_samplerDesc.AddressV       = convertToD3D12AddressMode(m_addressModeV);
    m_samplerDesc.AddressW       = convertToD3D12AddressMode(m_addressModeW);
    m_samplerDesc.MinLOD         = 0;
    m_samplerDesc.MaxLOD         = D3D12_FLOAT32_MAX;
    m_samplerDesc.MipLODBias     = 0.0f;
    m_samplerDesc.MaxAnisotropy  = 1;
    m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    m_heapPositionSampler =
        m_parentDevice->getCommonSamplerHeapManager()->addSampler(m_samplerDesc);
}

}   // namespace texture
}   // namespace renderer
