#include "renderer/scene/node_visitor/inc/ray_tracing_visitor.hpp"

namespace renderer {
namespace scene {

RayTracingVisitor::RayTracingVisitor()
{
    m_mask = VISITOR_MASK_RAYTRACE;
}

}   // namespace scene
}   // namespace renderer
