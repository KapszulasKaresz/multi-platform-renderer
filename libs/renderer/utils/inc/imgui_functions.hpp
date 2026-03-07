#ifndef IMGUI_FUNCTIONS_HPP_INCLUDED
#define IMGUI_FUNCTIONS_HPP_INCLUDED

#include "renderer/material/inc/material.hpp"
#include "renderer/mesh/inc/mesh.hpp"

namespace ImGui {
void DrawMesh(renderer::mesh::Mesh& f_mesh);
void DrawMaterial(renderer::material::Material& f_material);
}   // namespace ImGui

#endif
