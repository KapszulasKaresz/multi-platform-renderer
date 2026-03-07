#include "renderer/scene/inc/node_3d.hpp"

namespace renderer {
namespace scene {
Node3D::Node3D() : Node()
{
    m_name = "Node3D " + std::to_string(m_id);
}
}   // namespace scene
}   // namespace renderer
