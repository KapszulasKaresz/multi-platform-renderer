#include "renderer/scene/inc/test_scene.hpp"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    static auto l_startTime = std::chrono::high_resolution_clock::now();

    auto  l_currentTime = std::chrono::high_resolution_clock::now();
    float l_time        = std::chrono::duration<float, std::chrono::seconds::period>(
                       l_currentTime - l_startTime
    )
                       .count();

    auto l_uniformCollection = m_material->getUniformCollection("Camera");

    uniform::UniformSingle* l_modelUniform =
        dynamic_cast<uniform::UniformSingle*>(l_uniformCollection->getMember("model"));

    l_modelUniform->setValue(
        rotate(glm::mat4(1.0f), l_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
    );

    f_commandBuffer->useMaterial(m_material);
    f_commandBuffer->useViewport({ .m_fullScreen = true });
    f_commandBuffer->draw(m_mesh);
}
}   // namespace scene
}   // namespace renderer
