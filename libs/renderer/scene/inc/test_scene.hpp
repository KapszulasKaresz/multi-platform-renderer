#ifndef TEST_SCENE_HPP_INCLUDED
#define TEST_SCENE_HPP_INCLUDED

#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/scene/inc/node.hpp"

namespace renderer {
namespace command_buffer {
class CommandBuffer;
}   // namespace command_buffer

namespace material {
class Material;
}   // namespace material

namespace scene {
class TestScene : public RenderResource {
public:
    TestScene& setMaterial(std::shared_ptr<material::Material> f_material);
    TestScene& setMesh(std::shared_ptr<mesh::TriangleMesh> f_mesh);
    TestScene& create();

    void recordCommandBuffer(command_buffer::CommandBuffer* f_commandBuffer);

private:
    std::shared_ptr<material::Material> m_material{ nullptr };
    std::shared_ptr<mesh::TriangleMesh> m_mesh{ nullptr };

    std::unique_ptr<Node> m_rootNode{ nullptr };
};
}   // namespace scene
}   // namespace renderer

#endif
