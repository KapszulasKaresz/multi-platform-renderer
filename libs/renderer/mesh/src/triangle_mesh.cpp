#include "renderer/mesh/inc/triangle_mesh.hpp"

namespace renderer {
namespace mesh {
TriangleMesh& TriangleMesh::create()
{
    return *this;
}

int TriangleMesh::getIndicieCount() const
{
    return m_indices.size();
}
}   // namespace mesh
}   // namespace renderer
