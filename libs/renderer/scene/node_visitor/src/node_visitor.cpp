#include "renderer/scene/node_visitor/inc/node_visitor.hpp"

#include "renderer/scene/node/inc/gltf_node.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node/inc/node.hpp"
#include "renderer/scene/node/inc/node_3d.hpp"

namespace renderer {
namespace scene {
VisitorMask NodeVisitor::getMask() const
{
    return static_cast<VisitorMask>(m_mask);
}

void NodeVisitor::visit(Node& f_node)
{
    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void NodeVisitor::visit(Node3D& f_node)
{
    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void NodeVisitor::visit(MeshInstanceNode& f_meshInstanceNode)
{
    for (const auto& l_child : f_meshInstanceNode.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void NodeVisitor::visit(GltfNode& f_node)
{
    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

}   // namespace scene
}   // namespace renderer
