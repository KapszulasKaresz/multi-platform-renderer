#ifndef DESCRIPTOR_HEAP_MANAGER
#define DESCRIPTOR_HEAP_MANAGER

#include <stdexcept>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace utils {
class DescriptorHeapManagerDX {
public:
    DescriptorHeapManagerDX(
        rendering_device::RenderingDeviceDX* f_parentDevice,
        UINT                                 f_numDescriptors,
        D3D12_DESCRIPTOR_HEAP_TYPE           f_type
    );

    UINT addCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& f_cbvDesc);
    UINT addSRV(
        ID3D12Resource*                        f_resource,
        const D3D12_SHADER_RESOURCE_VIEW_DESC& f_srvDesc
    );
    UINT addSampler(const D3D12_SAMPLER_DESC& f_desc);

    UINT addEmpty();

    D3D12_GPU_DESCRIPTOR_HANDLE getGPUStart() const;
    D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(UINT f_index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(UINT f_index) const;

    ID3D12DescriptorHeap* getHeap() const;
    UINT                  getNumDescriptors() const;

    void free(UINT f_idx);
    void free(
        D3D12_CPU_DESCRIPTOR_HANDLE f_cpuDescHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE f_gpuDescHandle
    );

private:
    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap{ nullptr };
    UINT                                         m_descriptorSize{ 0 };
    std::vector<int>                             m_freeHandles{};

    D3D12_DESCRIPTOR_HEAP_TYPE m_type{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
};
}   // namespace utils
}   // namespace renderer

#endif
