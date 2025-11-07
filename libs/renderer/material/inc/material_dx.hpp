#ifndef MATERIAL_VULKAN_DX_INCLUDED
#define MATERIAL_VULKAN_DX_INCLUDED

#include <array>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace material {
class MaterialDX : public Material {
public:
    MaterialDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    MaterialDX& create() override final;

    ID3D12RootSignature*               getRootSignature();
    std::vector<ID3D12DescriptorHeap*> getDescriptorHeaps();
    ID3D12PipelineState*               getPipelineState();

private:
    void createRootSignature();
    void createPipelineState();

    static std::array<D3D12_INPUT_ELEMENT_DESC, 4> getInputElementDescription();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState{ nullptr };
};
}   // namespace material
}   // namespace  renderer


#endif
