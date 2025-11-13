#include "renderer/image/inc/image_dx.hpp"

#include <stdexcept>

#include <stb_image.h>

#include "renderer/command_buffer/inc/command_buffer_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace image {
ImageDX::ImageDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : Image(),
      m_parentDevice(f_parentDevice)
{}

ImageDX& ImageDX::create()
{
    throw std::logic_error("Function not yet implemented");
}

ImageDX& ImageDX::createFromFile(std::string_view f_path)
{
    if (isValid()) {
        throw std::runtime_error(
            "ImageDX::createFromFile(std::string_view f_path) you "
            "cannot create an already created image"
        );
    }

    int      l_texWidth, l_texHeight, l_texChannels;
    stbi_uc* l_pixels = stbi_load(
        f_path.data(), &l_texWidth, &l_texHeight, &l_texChannels, STBI_rgb_alpha
    );
    m_format = IMAGE_FORMAT_RGBA8_SRGB;
    m_size.x = l_texWidth;
    m_size.y = l_texHeight;

    D3D12_RESOURCE_DESC l_imgDesc = {};
    l_imgDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    l_imgDesc.Alignment           = 0;
    l_imgDesc.Width               = m_size.x;
    l_imgDesc.Height              = m_size.y;
    l_imgDesc.DepthOrArraySize    = 1;
    l_imgDesc.MipLevels           = 1;
    l_imgDesc.Format              = convertToDXFormat(m_format);
    l_imgDesc.SampleDesc.Count    = 1;
    l_imgDesc.SampleDesc.Quality  = 0;
    l_imgDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    l_imgDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_allocDesc = {};
    l_allocDesc.HeapType                 = D3D12_HEAP_TYPE_DEFAULT;

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocDesc,
            &l_imgDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            NULL,
            &m_image,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "ImageDX::createFromFile(...) failed to create image from file"
        );
    }
    UINT64 l_uploadBufferSize = GetRequiredIntermediateSize(m_image->GetResource(), 0, 1);

    D3D12_RESOURCE_DESC l_uploadBufferDesc = {};
    l_uploadBufferDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_uploadBufferDesc.Alignment           = 0;
    l_uploadBufferDesc.Width               = l_uploadBufferSize;
    l_uploadBufferDesc.Height              = 1;
    l_uploadBufferDesc.DepthOrArraySize    = 1;
    l_uploadBufferDesc.MipLevels           = 1;
    l_uploadBufferDesc.Format              = DXGI_FORMAT_UNKNOWN;
    l_uploadBufferDesc.SampleDesc.Count    = 1;
    l_uploadBufferDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    l_uploadBufferDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_uploadAllocDesc = {};
    l_uploadAllocDesc.HeapType                 = D3D12_HEAP_TYPE_UPLOAD;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> l_uploadAllocation{ nullptr };

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_uploadAllocDesc,
            &l_uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            NULL,
            &l_uploadAllocation,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "ImageDX::createFromFile(...) failed to create upload buffer"
        );
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT l_footprint      = {};
    UINT                               l_numRows        = 0;
    UINT64                             l_rowSizeInBytes = 0;
    UINT64                             l_totalBytes     = 0;

    m_parentDevice->getDevice()->GetCopyableFootprints(
        &l_imgDesc, 0, 1, 0, &l_footprint, &l_numRows, &l_rowSizeInBytes, &l_totalBytes
    );

    BYTE*       l_mappedData = nullptr;
    D3D12_RANGE l_range      = { 0, 0 };   // We don't intend to read from it
    l_uploadAllocation->GetResource()->Map(
        0, &l_range, reinterpret_cast<void**>(&l_mappedData)
    );

    BYTE*       l_destSlice = l_mappedData + l_footprint.Offset;
    const BYTE* l_srcSlice  = l_pixels;

    for (UINT y = 0; y < l_numRows; ++y) {
        memcpy(
            l_destSlice + y * l_footprint.Footprint.RowPitch,
            l_srcSlice + y * (m_size.x * 4),   // 4 bytes per pixel (RGBA)
            m_size.x * 4
        );
    }

    l_uploadAllocation->GetResource()->Unmap(0, nullptr);

    command_buffer::CommandBufferDX* l_commanBuffer =
        dynamic_cast<command_buffer::CommandBufferDX*>(
            m_parentDevice->createCommandBuffer().get()
        );
    l_commanBuffer->reset();
    l_commanBuffer->begin();
    l_commanBuffer->copyBuffer(
        l_uploadAllocation->GetResource(), m_image->GetResource(), l_footprint
    );
    l_commanBuffer->end();
    l_commanBuffer->submit();
    m_parentDevice->waitForGPU();

    stbi_image_free(l_pixels);

    m_valid = true;
    return *this;
}

ImageDX& ImageDX::createEmptyImage()
{
    D3D12_RESOURCE_DESC l_imgDesc = {};
    l_imgDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    l_imgDesc.Alignment           = 0;
    l_imgDesc.Width               = m_size.x;
    l_imgDesc.Height              = m_size.y;
    l_imgDesc.DepthOrArraySize    = 1;
    l_imgDesc.MipLevels           = 1;
    l_imgDesc.Format              = convertToDXFormat(m_format);
    l_imgDesc.SampleDesc.Count    = 1;
    l_imgDesc.SampleDesc.Quality  = 0;
    l_imgDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    l_imgDesc.Flags = isDepthImage() ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
                                     : D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_allocDesc = {};
    l_allocDesc.HeapType                 = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE l_depthOptimizedClearValue    = {};
    l_depthOptimizedClearValue.Format               = convertToDXFormat(m_format);
    l_depthOptimizedClearValue.DepthStencil.Depth   = 1.0f;
    l_depthOptimizedClearValue.DepthStencil.Stencil = 0;

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocDesc,
            &l_imgDesc,
            isDepthImage() ? D3D12_RESOURCE_STATE_DEPTH_WRITE
                           : D3D12_RESOURCE_STATE_COMMON,
            isDepthImage() ? &l_depthOptimizedClearValue : NULL,
            &m_image,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "ImageDX::createEmptyImage(...) failed to create image from file"
        );
    }
    m_valid = true;
    return *this;
}

ImageDX& ImageDX::setFormat(image::ImageFormat f_format)
{
    m_format = f_format;
    return *this;
}

ImageDX& ImageDX::setColorSpace(image::ColorSpace f_colorSpace)
{
    m_colorSpace = f_colorSpace;
    return *this;
}

ImageDX& ImageDX::setSize(const glm::ivec2& f_size)
{
    m_size = f_size;
    return *this;
}

ID3D12Resource* ImageDX::getResource()
{
    return m_image->GetResource();
}

DXGI_FORMAT ImageDX::convertToDXFormat(const ImageFormat f_format)
{
    switch (f_format) {
        case IMAGE_FORMAT_RGBA8: {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }

        case IMAGE_FORMAT_RGBA8_SRGB: {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }

        // Note: DirectX lacks a native 24-bit format.
        // We map to a 32-bit format and the alpha channel will be ignored.
        case IMAGE_FORMAT_RGB8: {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }

        case IMAGE_FORMAT_RGB8_SRGB: {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }

        case IMAGE_FORMAT_BGRA8: {
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        }

        case IMAGE_FORMAT_BGRA8_SRGB: {
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }

        case IMAGE_FORMAT_DEPTH: {
            return DXGI_FORMAT_D32_FLOAT;
        }

        case IMAGE_FORMAT_UNDEFINED:
        case IMAGE_FORMAT_MAX:
        default:                     {
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

bool ImageDX::isDepthImage() const
{
    return m_format == IMAGE_FORMAT_DEPTH;
}

inline UINT64 ImageDX::GetRequiredIntermediateSize(
    _In_ ID3D12Resource*                       f_pDestinationResource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT f_FirstSubresource,
    _In_range_(0, D3D12_REQ_SUBRESOURCES - f_FirstSubresource) UINT f_NumSubresources
)
{
    D3D12_RESOURCE_DESC Desc         = f_pDestinationResource->GetDesc();
    UINT64              RequiredSize = 0;

    ID3D12Device* pDevice;
    f_pDestinationResource->GetDevice(IID_PPV_ARGS(&pDevice));

    pDevice->GetCopyableFootprints(
        &Desc,
        f_FirstSubresource,
        f_NumSubresources,
        0,
        nullptr,
        nullptr,
        nullptr,
        &RequiredSize
    );

    pDevice->Release();

    return RequiredSize;
}

}   // namespace image
}   // namespace renderer
