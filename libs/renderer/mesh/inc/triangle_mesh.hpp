#ifndef TRIANGLE_MESH_HPP_INCLUDED
#define TRIANGLE_MESH_HPP_INCLUDED

#include "renderer/mesh/inc/mesh.hpp"

namespace renderer {
namespace mesh {

class TriangleMesh : public Mesh {
public:
    virtual TriangleMesh& create() override;

    int getIndicieCount() const;

protected:
    // Test data
    std::vector<Vertex> m_vertices{
        { { -0.5f, -0.5f, 0.0f },
         { 1.0f, 0.0f, 0.0f },
         { 1.0f, 0.0f },
         { 1.0f, 0.0f, 0.0f } },
        {  { 0.5f, -0.5f, 0.0f },
         { 1.0f, 0.0f, 0.0f },
         { 0.0f, 0.0f },
         { 0.0f, 1.0f, 0.0f } },
        {   { 0.5f, 0.5f, 0.0f },
         { 1.0f, 0.0f, 0.0f },
         { 0.0f, 1.0f },
         { 0.0f, 0.0f, 1.0f } },
        {  { -0.5f, 0.5f, 0.0f },
         { 1.0f, 0.0f, 0.0f },
         { 1.0f, 1.0f },
         { 1.0f, 1.0f, 1.0f } }
    };
    std::vector<uint32_t> m_indices{ 0, 1, 2, 2, 3, 0 };
};
}   // namespace mesh
}   // namespace renderer

#endif
