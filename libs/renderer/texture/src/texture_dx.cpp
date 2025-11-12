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
    m_valid = true;
    return *this;
}

ID3D12DescriptorHeap* TextureDX::getDescriptorHeap()
{
    return m_srvHeap.Get();
}

void TextureDX::createShaderResourceView()
{
    D3D12_DESCRIPTOR_HEAP_DESC l_heapDesc = {};
    l_heapDesc.NumDescriptors             = 1;
    l_heapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    l_heapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    l_heapDesc.NodeMask                   = 0;

    m_parentDevice->getDevice()->CreateDescriptorHeap(
        &l_heapDesc, IID_PPV_ARGS(&m_srvHeap)
    );

    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};
    l_srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_srvDesc.Format        = image::ImageDX::convertToDXFormat(m_image->getFormat());
    l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    l_srvDesc.Texture2D.MostDetailedMip     = 0;
    l_srvDesc.Texture2D.MipLevels           = m_image->getMipLevels();
    l_srvDesc.Texture2D.PlaneSlice          = 0;
    l_srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_CPU_DESCRIPTOR_HANDLE l_srvHandle =
        m_srvHeap->GetCPUDescriptorHandleForHeapStart();

    image::ImageDX* l_imageDX = dynamic_cast<image::ImageDX*>(m_image.get());

    if (!l_imageDX) {
        throw std::runtime_error(
            "TextureDX::createShaderResourceView() image wasn't correct"
        );
    }

    m_parentDevice->getDevice()->CreateShaderResourceView(
        l_imageDX->getResource(), &l_srvDesc, l_srvHandle
    );
}
}   // namespace texture
}   // namespace renderer
