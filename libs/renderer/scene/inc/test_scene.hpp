#ifndef TEST_SCENE_HPP_INCLUDED
#define TEST_SCENE_HPP_INCLUDED

#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/scene/camera/inc/perspective_camera.hpp"
#include "renderer/scene/node/inc/node.hpp"
#include "renderer/scene/node_visitor/inc/draw_visitor.hpp"
#include "renderer/scene/node_visitor/inc/imgui_visitor.hpp"
#include "renderer/scene/observer/inc/controlled_observer.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace command_buffer {
class CommandBuffer;
}   // namespace command_buffer
namespace render_target {
class RenderTarget;
}   // namespace render_target

namespace material {
class Material;
}   // namespace material

namespace scene {

class TestScene : public RenderResource {
public:
    TestScene();

    TestScene& setMesh(std::shared_ptr<mesh::TriangleMesh> f_mesh);
    TestScene& create();

    ~TestScene();

    void recordCommandBuffer(
        command_buffer::CommandBuffer* f_commandBuffer,
        float                          f_deltaTime
    );
    bool isRayTracingEnabled() const;
    void recordRayTracingCommandBuffer(
        command_buffer::CommandBuffer* f_commandBuffer,
        render_target::RenderTarget*    f_renderTarget,
        uint32_t                       f_width,
        uint32_t                       f_height
    );

private:
    std::shared_ptr<material::Material> m_material{ nullptr };
    std::shared_ptr<mesh::TriangleMesh> m_mesh{ nullptr };

    std::unique_ptr<Node>               m_rootNode{ nullptr };
    std::shared_ptr<ray_tracing::AccelerationStructure> m_accelerationStructure{
        nullptr
    };
    std::shared_ptr<material::Material> m_rayTracingMaterial{ nullptr };
    std::shared_ptr<render_target::RenderTarget> m_rayTracingRenderTarget{ nullptr };
    std::shared_ptr<uniform::UniformCollection> m_rayTracingCameraUniform{ nullptr };
    bool m_useRayTracing{ false };
    ImGuiVisitor                        m_imguiVisitor{};
    DrawVisitor                         m_drawVisitor{};
    std::unique_ptr<PerspectiveCamera>  m_camera{ nullptr };
    std::unique_ptr<ControlledObserver> m_observer{ nullptr };
};
}   // namespace scene
}   // namespace renderer

#endif
