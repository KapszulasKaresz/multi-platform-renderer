#include "renderer/uniform/inc/uniform_storage_buffer_dx.hpp"

#include "renderer/command_buffer/inc/command_buffer_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace uniform {
UniformStorageBufferDX::UniformStorageBufferDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : UniformStorageBuffer(),
      m_parentDevice(f_parentDevice)
{}

UniformStorageBufferDX& UniformStorageBufferDX::create()
{
    m_valid = true;
    return *this;
}

void UniformStorageBufferDX::setValueRaw(std::span<const std::byte> f_dataView)
{
    m_size = f_dataView.size();
    createBuffer();

    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Width               = getAlignedSize(m_size);
    l_resourceDesc.Height              = 1;
    l_resourceDesc.DepthOrArraySize    = 1;
    l_resourceDesc.MipLevels           = 1;
    l_resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
    l_resourceDesc.SampleDesc.Count    = 1;
    l_resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    l_resourceDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_allocDesc = {};
    l_allocDesc.HeapType                 = D3D12_HEAP_TYPE_UPLOAD;

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocDesc,
            &l_resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            NULL,
            &m_uploadAllocation,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error("Failed to create upload buffer");
    }

    void* mappedData = nullptr;
    m_uploadAllocation->GetResource()->Map(0, nullptr, &mappedData);
    memcpy(mappedData, f_dataView.data(), f_dataView.size());
    m_uploadAllocation->GetResource()->Unmap(0, nullptr);

    D3D12_RESOURCE_BARRIER l_barrier = {};
    l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_barrier.Transition.pResource   = m_buffer->GetResource();
    l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
    l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();

    auto l_commandBufferDx =
        dynamic_cast<command_buffer::CommandBufferDX*>(l_commandBuffer.get());

    l_commandBufferDx->reset();
    l_commandBufferDx->begin();

    auto l_commandList = l_commandBufferDx->getCommandList();

    l_commandList->ResourceBarrier(1, &l_barrier);

    l_commandList->CopyBufferRegion(
        m_buffer->GetResource(), 0, m_uploadAllocation->GetResource(), 0, f_dataView.size()
    );

    l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l_commandList->ResourceBarrier(1, &l_barrier);

    l_commandBufferDx->end();
    l_commandBufferDx->submit();
}

void UniformStorageBufferDX::getValueRaw(std::span<std::byte> f_outBuffer)
{
    D3D12_RESOURCE_DESC l_resourceDesc = m_buffer->GetResource()->GetDesc();
    l_resourceDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC l_allocDesc = {};
    l_allocDesc.HeapType                 = D3D12_HEAP_TYPE_READBACK;


    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocDesc,
            &l_resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            NULL,
            &m_readbackAllocation,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error("Failed to create readback buffer");
    }

    D3D12_RESOURCE_BARRIER l_barrier = {};
    l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_barrier.Transition.pResource   = m_buffer->GetResource();
    l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();

    auto l_commandBufferDx =
        dynamic_cast<command_buffer::CommandBufferDX*>(l_commandBuffer.get());

    l_commandBufferDx->reset();
    l_commandBufferDx->begin();
    auto l_commandList = l_commandBufferDx->getCommandList();

    l_commandList->ResourceBarrier(1, &l_barrier);

    l_commandList->CopyBufferRegion(
        m_readbackAllocation->GetResource(),
        0,
        m_buffer->GetResource(),
        0,
        std::min<UINT>(f_outBuffer.size(), getSize())
    );

    l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l_commandList->ResourceBarrier(1, &l_barrier);
    l_commandBufferDx->end();
    l_commandBufferDx->submit();

    void*       mappedData = nullptr;
    D3D12_RANGE readRange  = { 0, f_outBuffer.size() };
    m_readbackAllocation->GetResource()->Map(0, &readRange, &mappedData);

    memcpy(f_outBuffer.data(), mappedData, f_outBuffer.size());


    D3D12_RANGE writtenRange = { 0, 0 };
    m_readbackAllocation->GetResource()->Unmap(0, &writtenRange);
}

size_t UniformStorageBufferDX::getAlignment() const
{
    return 256;
}

UINT UniformStorageBufferDX::getHeapOffset()
{
    return m_heapPosition;
}

D3D12MA::Allocation* UniformStorageBufferDX::getBuffer()
{
    return m_buffer.Get();
}

void UniformStorageBufferDX::createBuffer()
{
    if (m_buffer) {
        m_buffer.Reset();
    }

    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Alignment           = 0;
    l_resourceDesc.Width               = getAlignedSize(getSize());
    l_resourceDesc.Height              = 1;
    l_resourceDesc.DepthOrArraySize    = 1;
    l_resourceDesc.MipLevels           = 1;
    l_resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
    l_resourceDesc.SampleDesc.Count    = 1;
    l_resourceDesc.SampleDesc.Quality  = 0;
    l_resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    l_resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};

    l_allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocationDesc,
            &l_resourceDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            NULL,
            &m_buffer,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "UniformStorageBufferDX::createBuffer() failed to create uniform buffer"
        );
    }
    m_buffer->SetName(L"Storage Buffer UAV Resource");

    D3D12_UNORDERED_ACCESS_VIEW_DESC l_uavDesc = {};
    l_uavDesc.Format                           = DXGI_FORMAT_UNKNOWN;
    l_uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
    l_uavDesc.Buffer.FirstElement              = 0;

    UINT elementStride = sizeof(float);

    l_uavDesc.Buffer.NumElements          = getSize() / elementStride;
    l_uavDesc.Buffer.StructureByteStride  = elementStride;
    l_uavDesc.Buffer.CounterOffsetInBytes = 0;
    l_uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

    m_heapPosition = m_parentDevice->getCommonDescriptorHeapManager()->addUAV(
        m_buffer->GetResource(), l_uavDesc
    );
}

size_t UniformStorageBufferDX::getAlignedSize(size_t f_size) const
{
    size_t l_alignment = getAlignment();
    return (f_size + l_alignment - 1) & ~(l_alignment - 1);
}

}   // namespace uniform
}   // namespace renderer
