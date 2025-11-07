#include "renderer/mesh/inc/triangle_mesh_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace mesh {
TriangleMeshDX::TriangleMeshDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : TriangleMesh(),
      m_parentDevice(f_parentDevice)
{}

TriangleMeshDX& TriangleMeshDX::create()
{
    createVertexBuffer();
    createIndexBuffer();
    m_valid = true;
    return *this;
}

D3D12_VERTEX_BUFFER_VIEW& TriangleMeshDX::getVertexBufferView()
{
    return m_vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW& TriangleMeshDX::getIndexBufferView()
{
    return m_indexBufferView;
}

void TriangleMeshDX::createVertexBuffer()
{
    const UINT l_vertexBufferSize = sizeof(Vertex) * m_vertices.size();

    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Alignment           = 0;
    l_resourceDesc.Width               = l_vertexBufferSize;
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
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
            NULL,
            &m_vertexBuffer,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "TriangleMeshDX::createVertexBuffer() failed to create vertex buffer"
        );
    }

    D3D12_RANGE l_emptyRange = { 0, 0 };
    void*       l_mappedPtr;
    if (FAILED(m_vertexBuffer->GetResource()->Map(0, &l_emptyRange, &l_mappedPtr))) {
        throw std::runtime_error(
            "TriangleMeshDX::createVertexBuffer() failed to map vertex buffer"
        );
    }

    memcpy(l_mappedPtr, m_vertices.data(), l_vertexBufferSize);

    m_vertexBuffer->GetResource()->Unmap(0, NULL);

    m_vertexBufferView.BufferLocation =
        m_vertexBuffer->GetResource()->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes   = l_vertexBufferSize;
}

void TriangleMeshDX::createIndexBuffer()
{
    const UINT l_indexBufferSize = sizeof(uint32_t) * m_indices.size();

    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Alignment           = 0;
    l_resourceDesc.Width               = l_indexBufferSize;
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
            D3D12_RESOURCE_STATE_INDEX_BUFFER,
            NULL,
            &m_indexBuffer,
            IID_NULL,
            NULL
        )))
    {
        throw std::runtime_error(
            "TriangleMeshDX::createIndexBuffer() failed to create index buffer"
        );
    }

    D3D12_RANGE l_emptyRange = { 0, 0 };
    void*       l_mappedPtr;
    if (FAILED(m_indexBuffer->GetResource()->Map(0, &l_emptyRange, &l_mappedPtr))) {
        throw std::runtime_error(
            "TriangleMeshDX::createIndexBuffer() failed to map index buffer"
        );
    }

    memcpy(l_mappedPtr, m_indices.data(), l_indexBufferSize);

    m_indexBuffer->GetResource()->Unmap(0, NULL);

    m_indexBufferView.BufferLocation =
        m_indexBuffer->GetResource()->GetGPUVirtualAddress();
    m_indexBufferView.Format      = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = l_indexBufferSize;
}
}   // namespace mesh
}   // namespace renderer
