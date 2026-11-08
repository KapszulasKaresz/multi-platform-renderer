#ifndef MATERIAL_VULKAN_DX_INCLUDED
#define MATERIAL_VULKAN_DX_INCLUDED

#include <array>
#include <climits>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <D3D12MemAlloc.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace image {
class ImageDX;
}

namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace material {
class MaterialDX : public Material {
public:
    MaterialDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    MaterialDX& create() override final;

    ID3D12RootSignature* getRootSignature();
    ID3D12PipelineState* getPipelineState();
    ID3D12StateObject* getRayTracingStateObject();

    std::vector<UINT> getCBVHeapOffsets();
    std::vector<UINT> getUAVHeapOffsets();
    std::vector<UINT> getSRVHeapOffsets();
    std::vector<UINT> getSamplerHeapOffsets();
    D3D12_DISPATCH_RAYS_DESC getDispatchRaysDesc(UINT f_width, UINT f_height) const;
    D3D12_RESOURCE_STATES getRayTracingOutputState(ID3D12Resource* f_resource) const;
    void setRayTracingOutputState(
        ID3D12Resource* f_resource, D3D12_RESOURCE_STATES f_state
    );
    void bindRayTracingResources(
        ID3D12GraphicsCommandList4* f_commandList, image::ImageDX* f_outputImage
    );

    ~MaterialDX();

private:
    void createRootSignature();
    void createRayTracingRootSignature();
    void createPipelineState();
    void createComputePipelineState();
    void createRayTracingPipelineState();
    void createRayTracingDescriptors();
    UINT getOrCreateOutputUAV(ID3D12Resource* f_resource, DXGI_FORMAT f_format);
    MaterialDX* getOriginalMaterial();
    const MaterialDX* getOriginalMaterial() const;

    static std::array<D3D12_INPUT_ELEMENT_DESC, 5> getInputElementDescription();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12StateObject>   m_rayTracingStateObject{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_shaderTable{ nullptr };
    D3D12_GPU_VIRTUAL_ADDRESS m_raygenShaderRecord{ 0 };
    D3D12_GPU_VIRTUAL_ADDRESS m_missShaderRecord{ 0 };
    D3D12_GPU_VIRTUAL_ADDRESS m_hitGroupShaderRecord{ 0 };

    UINT m_rayTracingGeometrySrv{ UINT_MAX };
    std::array<UINT, 3> m_rayTracingTextureSrvRanges{ UINT_MAX, UINT_MAX, UINT_MAX };
    std::array<UINT, 3> m_rayTracingSamplerRanges{ UINT_MAX, UINT_MAX, UINT_MAX };
    std::unordered_map<ID3D12Resource*, UINT> m_rayTracingOutputUavs;
    std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES>
        m_rayTracingOutputStates;
    std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES>
        m_rayTracingGeometryStates;
};
}   // namespace material
}   // namespace  renderer


#endif
