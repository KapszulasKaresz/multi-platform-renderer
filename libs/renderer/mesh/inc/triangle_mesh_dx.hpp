#ifndef TRIANGLE_MESH_DX_HPP_INCLUDED
#define TRIANGLE_MESH_DX_HPP_INCLUDED

#include "renderer/mesh/inc/triangle_mesh.hpp"

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
};
}   // namespace mesh
}   // namespace renderer

#endif
