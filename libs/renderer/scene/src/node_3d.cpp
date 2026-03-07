#include "renderer/scene/inc/node_3d.hpp"

namespace renderer {
namespace scene {
Node3D::Node3D() : Node()
{
    m_name = "Node3D " + std::to_string(m_id);
}

Node3D& Node3D::create()
{
    m_valid = true;
    return *this;
}

void Node3D::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

}   // namespace scene
}   // namespace renderer
