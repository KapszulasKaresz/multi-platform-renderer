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

size_t TriangleMesh::getVertexCount() const
{
    return m_vertices.size();
}

const std::vector<Vertex>& TriangleMesh::getVertices() const
{
    return m_vertices;
}

const std::vector<uint32_t>& TriangleMesh::getIndices() const
{
    return m_indices;
}

AABB TriangleMesh::getAABB()
{
    if (!m_aabbCalculated) {
        calculateAABB();
    }
    return m_aabb;
}

void TriangleMesh::calculateAABB()
{
    if (m_vertices.empty()) {
        m_aabb.m_min     = glm::vec3(0.0f);
        m_aabb.m_max     = glm::vec3(0.0f);
        m_aabbCalculated = true;
        return;
    }

    m_aabb.m_min = m_vertices[0].m_position;
    m_aabb.m_max = m_vertices[0].m_position;

    for (const auto& vertex : m_vertices) {
        m_aabb.m_min = glm::min(m_aabb.m_min, vertex.m_position);
        m_aabb.m_max = glm::max(m_aabb.m_max, vertex.m_position);
    }

    m_aabbCalculated = true;
}

TriangleMesh& TriangleMesh::setVertices(const std::vector<Vertex>& f_vertices)
{
    m_vertices       = f_vertices;
    m_aabbCalculated = false;
    return *this;
}

TriangleMesh& TriangleMesh::setIndices(const std::vector<uint32_t>& f_indices)
{
    m_indices        = f_indices;
    m_aabbCalculated = false;
    return *this;
}


}   // namespace mesh
}   // namespace renderer
