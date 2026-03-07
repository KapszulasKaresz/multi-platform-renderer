#include "renderer/scene/inc/test_scene.hpp"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"
#include "renderer/scene/inc/imgui_visitor.hpp"
#include "renderer/scene/inc/mesh_instance_node.hpp"

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
    m_rootNode = std::make_unique<Node>();
    m_rootNode->setName("Root Node").create();

    auto l_childNode1 = std::make_unique<Node3D>();
    l_childNode1->setName("Child Node 1").create();
    m_rootNode->addChild(std::move(l_childNode1));

    auto l_childNode2 = std::make_unique<MeshInstanceNode>();
    l_childNode2->setMesh(m_mesh).setMaterial(m_material).setName("Child Node 2").create();
    m_rootNode->addChild(std::move(l_childNode2));

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

    ImGui::Begin("Test Scene tree");
    m_rootNode->applyVisitor(&m_imguiVisitor);
    ImGui::End();
}
}   // namespace scene
}   // namespace renderer
