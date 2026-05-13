#ifndef UNIFORM_STORAGE_BUFFER_DX_HPP_INCLUDED
#define UNIFORM_STORAGE_BUFFER_DX_HPP_INCLUDED

#include <d3d12.h>
#include <D3D12MemAlloc.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/uniform/inc/uniform_storage_buffer.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace uniform {
class UniformStorageBufferDX : public UniformStorageBuffer {
public:
    UniformStorageBufferDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    UniformStorageBufferDX& create() override final;

    void setValueRaw(std::span<const std::byte> f_dataView) override;
    void getValueRaw(std::span<std::byte> f_outBuffer) override;

    size_t getAlignment() const override final;

    UINT getHeapOffset();

    D3D12MA::Allocation* getBuffer();

private:
    void   createBuffer();
    size_t getAlignedSize(size_t f_size) const;

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_uploadAllocation{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_readbackAllocation{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_buffer{ nullptr };
    UINT                                        m_heapPosition{ 0 };
};

}   // namespace uniform
}   // namespace renderer

#endif
