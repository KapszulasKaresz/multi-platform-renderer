#include "renderer/scene/node_visitor/inc/imgui_visitor.hpp"

#include <imgui.h>

#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node/inc/node.hpp"
#include "renderer/utils/inc/imgui_functions.hpp"

namespace renderer {
namespace scene {
#define IMGUI_VISIT_NODE(NODE, PREFIX)                              \
    {                                                               \
        ImGuiTreeNodeFlags l_flag = ImGuiTreeNodeFlags_DefaultOpen  \
                                  | ImGuiTreeNodeFlags_OpenOnArrow; \
                                                                    \
        if ((NODE).getChildCount() == 0) {                          \
            l_flag |= ImGuiTreeNodeFlags_Leaf;                      \
        }                                                           \
                                                                    \
        if (m_selected != nullptr && &(NODE) == m_selected) {       \
            l_flag |= ImGuiTreeNodeFlags_Selected;                  \
        }                                                           \
                                                                    \
        /* Zero-allocation*/                                        \
        bool l_isOpen = ImGui::TreeNodeEx(                          \
            (void*)(intptr_t)(NODE).getId(),                        \
            l_flag,                                                 \
            "%s%s",                                                 \
            PREFIX,                                                 \
            (NODE).getName().c_str()                                \
        );                                                          \
                                                                    \
        if (ImGui::IsItemClicked()) {                               \
            m_selected = &(NODE);                                   \
        }                                                           \
                                                                    \
        if (l_isOpen) {                                             \
            for (const auto& l_child : (NODE).getChildren()) {      \
                l_child->applyVisitor(this);                        \
            }                                                       \
            ImGui::TreePop();                                       \
        }                                                           \
    }

ImGuiVisitor::ImGuiVisitor() : NodeVisitor()
{
    m_mask = VISITOR_MASK_UI;
}

void ImGuiVisitor::visit(Node& f_node)
{
    IMGUI_VISIT_NODE(f_node, "Node | ")
}

void ImGuiVisitor::visit(Node3D& f_node)
{
    IMGUI_VISIT_NODE(f_node, "Node3D | ")
}

void ImGuiVisitor::visit(MeshInstanceNode& f_node){
    IMGUI_VISIT_NODE(f_node, "MeshInstanceNode | ")
}

Node* ImGuiVisitor::getSelectedNode() const
{
    return m_selected;
}

}   // namespace scene
}   // namespace renderer
