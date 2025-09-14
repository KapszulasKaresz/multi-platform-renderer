#ifndef COMMAND_BUFFER_HPP
#define COMMAND_BUFFER_HPP

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace mesh {
class TriangleMesh;
}   // namespace mesh

namespace material {
class Material;
}   // namespace material

namespace render_target {
class RenderTarget;
}   // namespace render_target

namespace command_buffer {
struct RenderBeginInfo {
    std::shared_ptr<render_target::RenderTarget> m_renderTarget{ nullptr };
    glm::vec4                                    m_clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
};

struct ViewportInfo {
    glm::vec2 m_positions{ 0.0f, 0.0f };
    glm::vec2 m_size{ 0.0f, 0.0f };

    // This ignores the m_size setting and uses the currently bound rendertarget size
    bool m_fullScreen{ false };
};

class CommandBuffer : public RenderResource {
public:
    virtual CommandBuffer& submit()                                                 = 0;
    virtual CommandBuffer& reset()                                                  = 0;
    virtual CommandBuffer& begin()                                                  = 0;
    virtual CommandBuffer& end()                                                    = 0;
    virtual CommandBuffer& beginRendering(const RenderBeginInfo& f_renderBeginInfo) = 0;
    virtual CommandBuffer& endRendering()                                           = 0;
    virtual CommandBuffer& useMaterial(std::shared_ptr<material::Material> f_material) = 0;
    virtual CommandBuffer& useViewport(const ViewportInfo& f_viewportInfo)  = 0;
    virtual CommandBuffer& draw(std::shared_ptr<mesh::TriangleMesh> f_mesh) = 0;
};
}   // namespace command_buffer
}   // namespace renderer


#endif
