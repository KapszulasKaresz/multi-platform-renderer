#include "renderer/utils/inc/descriptor_heap_manager_dx.hpp"

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace utils {

DescriptorHeapManagerDX::DescriptorHeapManagerDX(
    rendering_device::RenderingDeviceDX* f_parentDevice,
    UINT                                 f_numDescriptors
)
    : m_parentDevice(f_parentDevice)
{
    D3D12_DESCRIPTOR_HEAP_DESC l_heapDesc = {};
    l_heapDesc.NumDescriptors             = f_numDescriptors;
    l_heapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    l_heapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_heapDesc, IID_PPV_ARGS(&m_heap)
        )))
    {
        throw std::runtime_error("Failed to create combined descriptor heap");
    }

    m_descriptorSize = m_parentDevice->getDevice()->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
    m_nextFreeHandle = 0;
}

UINT DescriptorHeapManagerDX::addCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& f_cbvDesc)
{
    if (m_nextFreeHandle >= getNumDescriptors()) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::addCBV(...) Descriptor heap full"
        );
    }

    auto l_handle = getCPUHandle(m_nextFreeHandle);
    m_parentDevice->getDevice()->CreateConstantBufferView(&f_cbvDesc, l_handle);
    auto l_originalFreeHandle = m_nextFreeHandle;
    m_nextFreeHandle++;
    return l_originalFreeHandle;
}

UINT DescriptorHeapManagerDX::addSRV(
    ID3D12Resource*                        f_resource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC& f_srvDesc
)
{
    if (m_nextFreeHandle >= getNumDescriptors()) {
        throw std::runtime_error("Descriptor heap full");
    }

    auto l_handle = getCPUHandle(m_nextFreeHandle);
    m_parentDevice->getDevice()->CreateShaderResourceView(
        f_resource, &f_srvDesc, l_handle
    );
    auto l_originalFreeHandle = m_nextFreeHandle;
    m_nextFreeHandle++;
    return l_originalFreeHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapManagerDX::getGPUStart() const
{
    return m_heap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapManagerDX::getCPUHandle(UINT f_index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE l_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
    l_handle.ptr += f_index * m_descriptorSize;
    return l_handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapManagerDX::getGPUHandle(UINT f_index) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE l_handle = m_heap->GetGPUDescriptorHandleForHeapStart();
    l_handle.ptr += f_index * m_descriptorSize;
    return l_handle;
}

ID3D12DescriptorHeap* DescriptorHeapManagerDX::getHeap() const
{
    return m_heap.Get();
}

UINT DescriptorHeapManagerDX::getNumDescriptors() const
{
    return m_heap->GetDesc().NumDescriptors;
}

}   // namespace utils
}   // namespace  renderer
