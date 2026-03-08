#include "renderer/scene/inc/mesh_instance_node.hpp"

namespace renderer {
namespace scene {
MeshInstanceNode::MeshInstanceNode() : Node3D()
{
    m_name = "Mesh Instance Node " + std::to_string(m_id);
}

MeshInstanceNode& MeshInstanceNode::setMaterial(
    std::shared_ptr<material::Material> f_material
)
{
    m_material = f_material;
    return *this;
}

MeshInstanceNode& MeshInstanceNode::setMesh(std::shared_ptr<mesh::Mesh> f_mesh)
{
    m_mesh = f_mesh;
    return *this;
}

MeshInstanceNode& MeshInstanceNode::create()
{
    m_valid = true;
    return *this;
}

std::shared_ptr<material::Material> MeshInstanceNode::getMaterial()
{
    return m_material;
}

std::shared_ptr<mesh::Mesh> MeshInstanceNode::getMesh()
{
    return m_mesh;
}

void MeshInstanceNode::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

MeshInstanceNode::~MeshInstanceNode() {}

}   // namespace scene
}   // namespace renderer
