#include "renderer/scene/inc/test_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"
#include "renderer/scene/inc/imgui_visitor.hpp"
#include "renderer/scene/inc/mesh_instance_node.hpp"
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
