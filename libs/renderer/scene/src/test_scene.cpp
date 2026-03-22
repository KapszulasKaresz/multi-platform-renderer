#include "renderer/scene/inc/test_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"
#include "renderer/render_target/inc/render_target.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/scene/node/inc/gltf_node.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node_visitor/inc/imgui_visitor.hpp"
#include "renderer/utils/inc/imgui_functions.hpp"

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

    auto l_gltfNode = std::make_unique<GltfNode>();
    l_gltfNode->setName("GLTF Node").create();
    // l_gltfNode->loadFromFile("res/models/glTF-Sample-Models/2.0/Buggy/glTF/Buggy.gltf");
    l_gltfNode->loadFromFile("res/models/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
    m_rootNode->addChild(std::move(l_gltfNode));

    m_camera = std::make_unique<PerspectiveCamera>();
    m_drawVisitor.setCamera(m_camera.get());

    m_observer = std::make_unique<ControlledObserver>();
    m_observer->setWindow(rendering_server::RenderingServer::getInstance().getWindow());
    m_observer->setCamera(m_camera.get());

    m_valid = true;
    return *this;
}

void TestScene::recordCommandBuffer(
    command_buffer::CommandBuffer* f_commandBuffer,
    float                          f_deltaTime
)
{
    m_camera->setAspectRatio(
        static_cast<float>(f_commandBuffer->getCurrentRenderTarget()->getSize().x)
        / static_cast<float>(f_commandBuffer->getCurrentRenderTarget()->getSize().y)
    );

    m_observer->update(f_deltaTime);

    m_drawVisitor.setCommandBuffer(f_commandBuffer);
    m_rootNode->applyVisitor(&m_drawVisitor);

    ImGui::Begin("Test Scene tree");
    m_rootNode->applyVisitor(&m_imguiVisitor);
    ImGui::End();

    ImGui::Begin("Test Scene selected node");
    auto l_selectedNode = m_imguiVisitor.getSelectedNode();
    if (l_selectedNode) {
        ImGui::DrawNodeDynamic(*l_selectedNode);
    }
    ImGui::End();
}
}   // namespace scene
}   // namespace renderer
