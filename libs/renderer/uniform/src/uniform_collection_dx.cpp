#include "renderer/uniform/inc/uniform_collection_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/uniform/inc/uniform_single_dx.hpp"
#include "renderer/utils/inc/utils.hpp"

namespace renderer {
namespace uniform {
UniformCollectionDX::UniformCollectionDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : UniformCollection(),
      m_parentDevice(f_parentDevice)
{}

void UniformCollectionDX::update()
{
    D3D12_RANGE l_readRange = { 0, 0 };
    if (FAILED(m_constantBuffer->GetResource()->Map(
            0, &l_readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)
        )))
    {
        throw std::runtime_error("TriangleMeshDX::update() failed to map uniform buffer");
    }
    memcpy(m_mappedConstantBuffer, createCPUBuffer().data(), getSize());
    m_constantBuffer->GetResource()->Unmap(0, &l_readRange);
}

size_t UniformCollectionDX::getSize() const
{
    return m_layout.m_structSize;
}

size_t UniformCollectionDX::getAlignment() const
{
    return 256;
}

UniformSingle* UniformCollectionDX::addMember(const std::string& f_name)
{
    m_members.push_back(std::make_unique<UniformSingleDX>());
    m_members.back()->setName(f_name);
    return dynamic_cast<UniformSingle*>(m_members.back().get());
}

UniformCollectionDX& UniformCollectionDX::create()
{
    computeLayout();
    createBuffer();
    m_valid = true;
    return *this;
}

void UniformCollectionDX::createBuffer()
{
    D3D12_DESCRIPTOR_HEAP_DESC l_heapDesc = {};
    l_heapDesc.NumDescriptors             = 1;
    l_heapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    l_heapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_heapDesc, IID_PPV_ARGS(&m_constantBufferHeap)
        )))
    {
        throw std::runtime_error(
            "UniformCollectionDX::createBuffer() failed to create descriptor heap"
        );
    }
    m_constantBufferHeap->SetName(L"Constant Buffer Descriptor Heap");

    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Alignment           = 0;
    l_resourceDesc.Width               = getSize();
    l_resourceDesc.Height              = 1;
    l_resourceDesc.DepthOrArraySize    = 1;
    l_resourceDesc.MipLevels           = 1;
    l_resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
    l_resourceDesc.SampleDesc.Count    = 1;
    l_resourceDesc.SampleDesc.Quality  = 0;
    l_resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    l_resourceDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};
    l_allocationDesc.HeapType                 = D3D12_HEAP_TYPE_UPLOAD;

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocationDesc,
            &l_resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            NULL,
            &m_constantBuffer,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "TriangleMeshDX::createBuffer() failed to create uniform buffer"
        );
    }
    m_constantBuffer->SetName(L"Constant Buffer Upload Resource");

    m_constantBufferView.BufferLocation =
        m_constantBuffer->GetResource()->GetGPUVirtualAddress();
    m_constantBufferView.SizeInBytes = getSize();

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_cbvHandle(m_constantBufferHeap->GetCPUDescriptorHandleForHeapStart());
    l_cbvHandle.ptr = l_cbvHandle.ptr
                    + m_parentDevice->getDevice()->GetDescriptorHandleIncrementSize(
                          D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                      ) * 0;
}

void UniformCollectionDX::computeLayout()
{
    size_t l_offset   = 0;
    size_t l_maxAlign = 16;   // Each member aligned to 16 bytes (float4)

    m_layout.m_offsets.clear();

    size_t l_registerOffset = 0;   // offset within a 16-byte register

    for (auto& l_member : m_members) {
        size_t l_size  = l_member->getSize();
        size_t l_align = std::min<size_t>(l_member->getAlignment(), 16);

        // If it doesn't fit in current 16-byte slot, move to next
        if (l_registerOffset + l_size > 16 || l_align == 16) {
            l_offset         = utils::alignUp(l_offset, 16);
            l_registerOffset = 0;
        }

        m_layout.m_offsets.push_back(l_offset + l_registerOffset);

        l_registerOffset += l_size;

        // Advance to next 16-byte block if filled
        if (l_registerOffset >= 16) {
            l_offset         = utils::alignUp(l_offset + l_registerOffset, 16);
            l_registerOffset = 0;
        }
    }

    // Struct size rounded up to nearest 16
    size_t l_structSize = utils::alignUp(l_offset + l_registerOffset, 16);

    m_layout.m_structSize = utils::alignUp(l_structSize, 256);
}

std::vector<uint8_t> UniformCollectionDX::createCPUBuffer()
{
    std::vector<uint8_t> l_buffer(m_layout.m_structSize, 0);

    for (size_t i = 0; i < m_members.size(); i++) {
        if (m_members[i]->getType() != UniformType::UNIFORM_TYPE_STRUCT) {
            uniform::UniformSingle* l_rawMember =
                dynamic_cast<uniform::UniformSingle*>(m_members[i].get());
            std::memcpy(
                l_buffer.data() + m_layout.m_offsets[i],
                l_rawMember->valueAsVoid(),
                m_members[i]->getSize()
            );
        }
    }

    return l_buffer;
}

}   // namespace uniform
}   // namespace renderer
