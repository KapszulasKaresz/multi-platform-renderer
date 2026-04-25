#include "renderer/mesh/inc/triangle_mesh.hpp"

namespace renderer {
namespace mesh {
TriangleMesh& TriangleMesh::create()
{
    m_valid = true;
    return *this;
}

int TriangleMesh::getIndicieCount() const
{
    return m_indices.size();
}

TriangleMesh& TriangleMesh::setVertices(const std::vector<Vertex>& f_vertices)
{
    m_vertices = f_vertices;
    return *this;
}

TriangleMesh& TriangleMesh::setIndices(const std::vector<uint32_t>& f_indices)
{
    m_indices = f_indices;
    return *this;
}

}   // namespace mesh
}   // namespace renderer
