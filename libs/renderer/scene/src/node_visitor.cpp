#include "renderer/scene/inc/node_visitor.hpp"

namespace renderer {
namespace scene {
VisitorMask NodeVisitor::getMask() const
{
    return static_cast<VisitorMask>(m_mask);
}
}   // namespace scene
}   // namespace renderer
