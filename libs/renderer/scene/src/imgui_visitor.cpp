#include "renderer/scene/inc/imgui_visitor.hpp"

#include <imgui.h>

#include "renderer/scene/inc/node.hpp"

namespace renderer {
namespace scene {
ImGuiVisitor::ImGuiVisitor() : NodeVisitor()
{
    m_mask = VISITOR_MASK_UI;
}

void ImGuiVisitor::visit(Node& f_node)
{
    if (ImGui::TreeNode(f_node.getName().c_str())) {
        for (const auto& l_child : f_node.getChildren()) {
            l_child->applyVisitor(this);
        }

        ImGui::TreePop();
    }
}
}   // namespace scene
}   // namespace renderer
