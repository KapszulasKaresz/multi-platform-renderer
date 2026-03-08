#ifndef MESH_INSTANCE_NODE_HPP
#define MESH_INSTANCE_NODE_HPP

#include "renderer/scene/inc/node_3d.hpp"

namespace renderer {
namespace material {
class Material;
}   // namespace material

namespace mesh {
class Mesh;
}   // namespace mesh

namespace scene {
class MeshInstanceNode : public Node3D {
public:
    MeshInstanceNode();

    MeshInstanceNode& setMaterial(std::shared_ptr<material::Material> f_material);
    MeshInstanceNode& setMesh(std::shared_ptr<mesh::Mesh> f_mesh);
    MeshInstanceNode& create();

    std::shared_ptr<material::Material> getMaterial();
    std::shared_ptr<mesh::Mesh>         getMesh();


    virtual void applyVisitor(NodeVisitor* f_visitor) override;

    ~MeshInstanceNode();

protected:
    std::shared_ptr<material::Material> m_material{ nullptr };
    std::shared_ptr<mesh::Mesh>         m_mesh{ nullptr };
};
}   // namespace scene
}   // namespace renderer

#endif
