#include "renderer/utils/inc/imgui_functions.hpp"

#include <imgui.h>

namespace ImGui {
void DrawMesh(renderer::mesh::Mesh& f_mesh)
{
    ImGui::Text("Mesh");
}

void DrawMaterial(renderer::material::Material& f_material)
{
    ImGui::Text("Material");
}
}   // namespace ImGui
