#include "renderer/scene/inc/test_scene.hpp"

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace scene {
TestScene& TestScene::setMaterial(std::shared_ptr<material::Material> f_material)
{
    m_material = f_material;
    return *this;
}

TestScene& TestScene::setMesh(std::shared_ptr<mesh::TriangleMesh> f_mesh)
{
    m_mesh = f_mesh;
    return *this;
}

TestScene& TestScene::create()
{
    m_valid = true;
    return *this;
}

void TestScene::recordCommandBuffer(command_buffer::CommandBuffer* f_commandBuffer)
{
    f_commandBuffer->useMaterial(m_material);
    f_commandBuffer->useViewport({ .m_fullScreen = true });
    f_commandBuffer->draw(m_mesh);
}
}   // namespace scene
}   // namespace renderer
