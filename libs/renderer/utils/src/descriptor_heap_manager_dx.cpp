#include "renderer/utils/inc/descriptor_heap_manager_dx.hpp"

#include <algorithm>
#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace utils {

DescriptorHeapManagerDX::DescriptorHeapManagerDX(
    rendering_device::RenderingDeviceDX* f_parentDevice,
    UINT                                 f_numDescriptors,
    D3D12_DESCRIPTOR_HEAP_TYPE           f_type
)
    : m_parentDevice(f_parentDevice),
      m_type(f_type)
{
    D3D12_DESCRIPTOR_HEAP_DESC l_heapDesc = {};
    l_heapDesc.NumDescriptors             = f_numDescriptors;
    l_heapDesc.Type                       = m_type;
    l_heapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(m_parentDevice->getDevice()->CreateDescriptorHeap(
            &l_heapDesc, IID_PPV_ARGS(&m_heap)
        )))
    {
        throw std::runtime_error("Failed to create combined descriptor heap");
    }

    m_descriptorSize = m_parentDevice->getDevice()->GetDescriptorHandleIncrementSize(
        m_type
    );
    m_freeHandles.reserve(l_heapDesc.NumDescriptors);
    for (int n = l_heapDesc.NumDescriptors; n > 0; n--) {
        m_freeHandles.push_back(n - 1);
    }
}

UINT DescriptorHeapManagerDX::addCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& f_cbvDesc)
{
    if (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::addCBV() type must be "
            "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV"
        );
    }
    auto l_originalFreeHandle = allocateRange(1);
    auto l_handle             = getCPUHandle(l_originalFreeHandle);
    m_parentDevice->getDevice()->CreateConstantBufferView(&f_cbvDesc, l_handle);
    return l_originalFreeHandle;
}

UINT DescriptorHeapManagerDX::addUAV(
    ID3D12Resource*                         f_resource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC& f_uavDesc
)
{
    if (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::addUAV() type must be "
            "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV"
        );
    }
    auto l_originalFreeHandle = allocateRange(1);
    auto l_handle             = getCPUHandle(l_originalFreeHandle);

    m_parentDevice->getDevice()->CreateUnorderedAccessView(
        f_resource, nullptr, &f_uavDesc, l_handle
    );

    return l_originalFreeHandle;
}

UINT DescriptorHeapManagerDX::addSRV(
    ID3D12Resource*                        f_resource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC& f_srvDesc
)
{
    if (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::addSRV() type must be "
            "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV"
        );
    }
    auto l_originalFreeHandle = allocateRange(1);
    auto l_handle             = getCPUHandle(l_originalFreeHandle);
    m_parentDevice->getDevice()->CreateShaderResourceView(
        f_resource, &f_srvDesc, l_handle
    );
    return l_originalFreeHandle;
}

UINT DescriptorHeapManagerDX::addSampler(const D3D12_SAMPLER_DESC& f_desc)
{
    if (m_type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::addSampler() type must be "
            "D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER"
        );
    }
    auto l_originalFreeHandle = allocateRange(1);
    auto l_handle             = getCPUHandle(l_originalFreeHandle);
    m_parentDevice->getDevice()->CreateSampler(&f_desc, l_handle);
    return l_originalFreeHandle;
}

UINT DescriptorHeapManagerDX::addEmpty()
{
    return allocateRange(1);
}

UINT DescriptorHeapManagerDX::allocateRange(UINT f_count)
{
    if (f_count == 0 || f_count > m_freeHandles.size()) {
        throw std::runtime_error(
            "DescriptorHeapManagerDX::allocateRange() invalid descriptor count"
        );
    }

    for (UINT l_candidate = 0;
         l_candidate + f_count <= m_heap->GetDesc().NumDescriptors;
         ++l_candidate)
    {
        std::vector<UINT> l_range;
        l_range.reserve(f_count);
        for (UINT i = 0; i < f_count; ++i) {
            const auto l_handle = l_candidate + i;
            if (std::find(m_freeHandles.begin(), m_freeHandles.end(), l_handle)
                == m_freeHandles.end())
            {
                break;
            }
            l_range.push_back(l_handle);
        }
        if (l_range.size() == f_count) {
            for (const auto l_handle : l_range) {
                m_freeHandles.erase(
                    std::find(m_freeHandles.begin(), m_freeHandles.end(), l_handle)
                );
            }
            return l_candidate;
        }
    }

    throw std::runtime_error(
        "DescriptorHeapManagerDX::allocateRange() descriptor heap full or fragmented"
    );
}

void DescriptorHeapManagerDX::freeRange(UINT f_first, UINT f_count)
{
    for (UINT i = 0; i < f_count; ++i) {
        free(f_first + i);
    }
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

void DescriptorHeapManagerDX::free(UINT f_idx)
{
    m_freeHandles.push_back(f_idx);
}

void DescriptorHeapManagerDX::free(
    D3D12_CPU_DESCRIPTOR_HANDLE f_cpuDescHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE f_gpuDescHandle
)
{
    int l_cpuIdx =
        (int)((f_cpuDescHandle.ptr - m_heap->GetCPUDescriptorHandleForHeapStart().ptr)
              / m_descriptorSize);
    int l_gpuIdx =
        (int)((f_gpuDescHandle.ptr - m_heap->GetGPUDescriptorHandleForHeapStart().ptr)
              / m_descriptorSize);

    if (l_cpuIdx != l_gpuIdx) {
        throw std::
            runtime_error(
                "DescriptorHeapManagerDX::free(...) provided cpu and gpu handled aren't "
                "for " "the same resource"
            );
    }

    free(l_cpuIdx);
}

}   // namespace utils
}   // namespace  renderer
