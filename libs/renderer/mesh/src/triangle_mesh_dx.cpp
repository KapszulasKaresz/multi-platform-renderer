#include "renderer/mesh/inc/triangle_mesh_dx.hpp"

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

void TriangleMeshDX::createVertexBuffer() {}

void TriangleMeshDX::createIndexBuffer() {}
}   // namespace mesh
}   // namespace renderer
