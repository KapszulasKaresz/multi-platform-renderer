#ifndef TRIANGLE_MESH_HPP_INCLUDED
#define TRIANGLE_MESH_HPP_INCLUDED

#include "renderer/mesh/inc/mesh.hpp"

namespace renderer {
namespace mesh {

class TriangleMesh : public Mesh {
public:
    virtual TriangleMesh& create() override;

    TriangleMesh& setVertices(const std::vector<Vertex>& f_vertices);
    TriangleMesh& setIndices(const std::vector<uint32_t>& f_indices);

    size_t getVertexCount() const;
    int  getIndicieCount() const;
    AABB getAABB() override;

protected:
    void calculateAABB();

    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_indices;
    AABB                  m_aabb{};
    bool                  m_aabbCalculated{ false };
};
}   // namespace mesh
}   // namespace renderer

#endif
