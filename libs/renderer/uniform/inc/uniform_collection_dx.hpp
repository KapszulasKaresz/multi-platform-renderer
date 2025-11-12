#ifndef UNIFORM_COLLECTION_DX_HPP_INCLUDED
#define UNIFORM_COLLECTION_DX_HPP_INCLUDED

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/uniform/inc/uniform_collection.hpp"

#include "D3D12MemAlloc.h"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace uniform {
class UniformCollectionDX : public UniformCollection {
public:
    UniformCollectionDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    void   update() override final;
    size_t getSize() const override final;

    size_t         getAlignment() const override final;
    UniformSingle* addMember(const std::string& f_name) override final;

    UniformCollectionDX&               create() override final;
    ID3D12DescriptorHeap*              getDescriptorHeap();
    std::vector<ID3D12DescriptorHeap*> getDescriptorHeapSPV();

private:
    void createBuffer();
    void computeLayout();

    std::vector<uint8_t> createCPUBuffer();

    struct Layout {
        size_t              m_structSize;
        std::vector<size_t> m_offsets;
    } m_layout;

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<D3D12MA::Allocation>  m_constantBuffer{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_constantBufferHeap{ nullptr };
    D3D12_CONSTANT_BUFFER_VIEW_DESC              m_constantBufferView;
    UINT8*                                       m_mappedConstantBuffer{ nullptr };
};

}   // namespace uniform
}   // namespace renderer

#endif
