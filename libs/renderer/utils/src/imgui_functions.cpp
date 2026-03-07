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

void DrawMesh(renderer::mesh::Mesh& f_mesh)
{
    ImGui::Text("Mesh");
}

void DrawMaterial(renderer::material::Material& f_material)
{
    ImGui::Text("Material");
}
}   // namespace ImGui
