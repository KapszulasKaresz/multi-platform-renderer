#include "renderer/utils/inc/imgui_functions.hpp"

#include <imgui.h>

namespace ImGui {

void DrawNodeDynamic(renderer::scene::Node& f_node)
{
    if (auto* l_meshInstanceNode =
            dynamic_cast<renderer::scene::MeshInstanceNode*>(&f_node))
    {
        DrawNode(*l_meshInstanceNode);
        return;
    }
    else if (auto* l_gltfNode = dynamic_cast<renderer::scene::GltfNode*>(&f_node)) {
        DrawNode(*l_gltfNode);
        return;
    }
    else if (auto* l_node3D = dynamic_cast<renderer::scene::Node3D*>(&f_node)) {
        DrawNode(*l_node3D);
        return;
    }
    else {
        DrawNode(f_node);
        return;
    }
}

void DrawNode(renderer::scene::Node& f_node)
{
    if (ImGui::CollapsingHeader("Node")) {
    }
}

void DrawNode(renderer::scene::Node3D& f_node)
{
    if (ImGui::CollapsingHeader("Node3D")) {
        glm::vec3 l_translation = f_node.getTranslation();
        ImGui::SliderFloat3("Translation", &l_translation.x, -10.0f, 10.0f);
        f_node.setTranslation(l_translation);

        glm::vec3 l_rotationEuler = glm::degrees(glm::eulerAngles(f_node.getRotation()));
        ImGui::SliderFloat3(
            "Rotation (Euler angles)", &l_rotationEuler.x, -180.0f, 180.0f
        );
        f_node.setRotation(glm::quat(glm::radians(l_rotationEuler)));

        glm::vec3 l_scale = f_node.getScale();
        ImGui::SliderFloat3("Scale", &l_scale.x, -10.0f, 10.0f);
        f_node.setScale(l_scale);
    }
    DrawNode(static_cast<renderer::scene::Node&>(f_node));
}

void DrawNode(renderer::scene::MeshInstanceNode& f_node)
{
    if (ImGui::CollapsingHeader("MeshInstanceNode")) {
        if (f_node.getMesh()) {
            DrawMesh(*f_node.getMesh());
        }

        if (f_node.getMaterial()) {
            DrawMaterial(*f_node.getMaterial());
        }
    }
    DrawNode(static_cast<renderer::scene::Node3D&>(f_node));
}

void DrawNode(renderer::scene::GltfNode& f_node)
{
    if (ImGui::CollapsingHeader("GltfNode")) {
    }
    DrawNode(static_cast<renderer::scene::Node3D&>(f_node));
}

void DrawMesh(renderer::mesh::Mesh& f_mesh)
{
    ImGui::Text("Mesh");
}

void DrawMaterial(renderer::material::Material& f_material)
{
    ImGui::Text("Material");
}
}   // namespace ImGui
