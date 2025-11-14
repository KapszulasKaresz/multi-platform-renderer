#include "renderer/render_target/inc/render_target_dx.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/rendering_api/inc/rendering_api_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace render_target {
RenderTargetDX::RenderTargetDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : RenderTarget(),
      m_parentDevice(f_parentDevice)
{}

std::shared_ptr<image::Image> RenderTargetDX::getImage()
{
    return m_colorImage;
}

std::shared_ptr<image::Image> RenderTargetDX::getDepthImage()
{
    return m_depthImage;
}

glm::ivec2 RenderTargetDX::getSize() const
{
    return m_size;
}

RenderTargetDX& RenderTargetDX::create()
{
    validateMSAARequirements();
    createColorResource();
    if (m_useDepthBuffer) {
        createDepthResource();
    }
    createDescriptorHeap();
    createRenderTargets();
    m_valid = true;
    return *this;
}

ID3D12DescriptorHeap* RenderTargetDX::getMSAADescriptorHeap()
{
    return m_msaaRtvHeap.Get();
}

ID3D12DescriptorHeap* RenderTargetDX::getDepthDescriptorHeap()
{
    return m_dsvHeap.Get();
}

bool RenderTargetDX::isMSAA()
{
    return m_MSAASampleDesc.Count > 1;
}

void RenderTargetDX::createDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC l_dsvHeapDesc = {};
    l_dsvHeapDesc.NumDescriptors             = 1;
    l_dsvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    l_dsvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)
        )))
    {
        throw std::
            runtime_error(
                "RenderingDeviceDX::createDescriptorHeap() failed to create depth "
                "descriptor " "heap"
            );
    }

    D3D12_DESCRIPTOR_HEAP_DESC l_msaaRtvHeapDesc = {};
    l_msaaRtvHeapDesc.NumDescriptors             = 1;
    l_msaaRtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    l_msaaRtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_msaaRtvHeapDesc, IID_PPV_ARGS(&m_msaaRtvHeap)
        )))
    {
        throw std::
            runtime_error(
                "RenderingDeviceDX::createDescriptorHeap() failed to create color "
                "descriptor " "heap"
            );
    }
}

void RenderTargetDX::createRenderTargets()
{
    if (m_useDepthBuffer) {
        D3D12_DEPTH_STENCIL_VIEW_DESC l_dsvDesc = {};
        l_dsvDesc.Format = image::ImageDX::convertToDXFormat(m_depthImage->getFormat());
        l_dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        l_dsvDesc.Flags         = D3D12_DSV_FLAG_NONE;

        m_parentDevice->getDevice()->CreateDepthStencilView(
            m_depthImage->getResource(),
            &l_dsvDesc,
            m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
        );
    }

    D3D12_RENDER_TARGET_VIEW_DESC l_msaaRtvDesc = {};
    l_msaaRtvDesc.Format = image::ImageDX::convertToDXFormat(m_colorImage->getFormat());
    l_msaaRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

    m_parentDevice->getDevice()->CreateRenderTargetView(
        m_colorImage->getResource(),
        &l_msaaRtvDesc,
        m_msaaRtvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}

void RenderTargetDX::createDepthResource()
{
    m_depthImage =
        std::dynamic_pointer_cast<image::ImageDX>(m_parentDevice->createImage());
    m_depthImage->setFormat(image::IMAGE_FORMAT_DEPTH)
        .setColorSpace(image::COLOR_SPACE_LINEAR)
        .setSize(m_size)
        .setMSAASamples(m_MSAASampleDesc)
        .setDefaultState(D3D12_RESOURCE_STATE_DEPTH_WRITE)
        .setResrouceFlags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        .createEmptyImage();
}

void RenderTargetDX::createColorResource()
{
    m_colorImage =
        std::dynamic_pointer_cast<image::ImageDX>(m_parentDevice->createImage());
    m_colorImage->setFormat(m_format)
        .setColorSpace(image::COLOR_SPACE_LINEAR)
        .setSize(m_size)
        .setMSAASamples(m_MSAASampleDesc)
        .setDefaultState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        .setResrouceFlags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        .createEmptyImage();
}

void RenderTargetDX::validateMSAARequirements()
{
    m_MSAASampleDesc.Count   = m_parentDevice->getMaxMSAASamples();
    m_MSAASampleDesc.Quality = 0;

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS l_qualityLevels = {};
    l_qualityLevels.Format      = image::ImageDX::convertToDXFormat(m_format);
    l_qualityLevels.SampleCount = m_MSAASampleDesc.Count;

    if (FAILED(m_parentDevice->getDevice()->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &l_qualityLevels,
            sizeof(l_qualityLevels)
        ))
        || l_qualityLevels.NumQualityLevels == 0)
    {
        throw std::runtime_error(
            "RenderTargetDX::validateMSAARequirements() MSAA sample count not " "supporte"
                                                                                "d"
        );
    }

    m_MSAASampleDesc.Quality = l_qualityLevels.NumQualityLevels - 1;
}
}   // namespace render_target
}   // namespace renderer
