#include "renderer/scene/node_visitor/inc/ray_tracing_visitor.hpp"

#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"

namespace renderer {
namespace scene {

RayTracingVisitor::RayTracingVisitor()
{
    m_mask = VISITOR_MASK_RAYTRACE;
}

void RayTracingVisitor::visit(MeshInstanceNode& f_meshInstanceNode)
{
    if (f_meshInstanceNode.getMesh()) {
        auto l_mesh = std::dynamic_pointer_cast<mesh::TriangleMesh>(
            f_meshInstanceNode.getMesh()
        );
        if (l_mesh) {
            m_meshInstances.push_back({
                .m_mesh       = std::move(l_mesh),
                .m_transform  = f_meshInstanceNode.M(),
                .m_instanceId = static_cast<uint32_t>(f_meshInstanceNode.getId())
            });
        }
    }

    NodeVisitor::visit(f_meshInstanceNode);
}

const std::vector<RayTracingVisitor::MeshInstance>&
    RayTracingVisitor::getMeshInstances() const
{
    return m_meshInstances;
}

}   // namespace scene
}   // namespace renderer
