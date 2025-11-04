#ifndef TRIANGLE_MESH_DX_HPP_INCLUDED
#define TRIANGLE_MESH_DX_HPP_INCLUDED

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/mesh/inc/triangle_mesh.hpp"

#include "D3D12MemAlloc.h"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace mesh {

class TriangleMeshDX : public TriangleMesh {
public:
    TriangleMeshDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    TriangleMeshDX& create() override final;

protected:
    void createVertexBuffer();
    void createIndexBuffer();

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_vertexBuffer{ nullptr };
    D3D12_VERTEX_BUFFER_VIEW                    m_vertexBufferView;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_indexBuffer{ nullptr };
    D3D12_INDEX_BUFFER_VIEW                     m_indexBufferView;
};
}   // namespace mesh
}   // namespace renderer

#endif
