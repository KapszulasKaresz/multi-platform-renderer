#ifndef IMGUI_FUNCTIONS_HPP_INCLUDED
#define IMGUI_FUNCTIONS_HPP_INCLUDED

#include "renderer/material/inc/material.hpp"
#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/scene/inc/mesh_instance_node.hpp"

namespace ImGui {
// TODO: This is a temporary solution dynamic dispatching of ImGui drawing functions
// should be handled by a dedicated visitor
void DrawNodeDynamic(renderer::scene::Node& f_node);

void DrawNode(renderer::scene::Node& f_node);
void DrawNode(renderer::scene::Node3D& f_node);
void DrawNode(renderer::scene::MeshInstanceNode& f_node);

void DrawMesh(renderer::mesh::Mesh& f_mesh);
void DrawMaterial(renderer::material::Material& f_material);
}   // namespace ImGui

#endif
