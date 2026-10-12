#ifndef RAY_TRACING_VISITOR_HPP_INCLUDED
#define RAY_TRACING_VISITOR_HPP_INCLUDED

#include "renderer/scene/node_visitor/inc/node_visitor.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace renderer {
namespace mesh {
class TriangleMesh;
}

namespace scene {

class RayTracingVisitor : public NodeVisitor {
public:
    RayTracingVisitor();

    struct MeshInstance {
        std::shared_ptr<mesh::TriangleMesh> m_mesh;
        glm::mat4                           m_transform{ 1.0f };
        uint32_t                            m_instanceId{ 0 };
    };

    void visit(MeshInstanceNode& f_meshInstanceNode) override;

    const std::vector<MeshInstance>& getMeshInstances() const;

    virtual ~RayTracingVisitor() = default;

protected:
    std::vector<MeshInstance> m_meshInstances;
};

}   // namespace scene
}   // namespace renderer

#endif
