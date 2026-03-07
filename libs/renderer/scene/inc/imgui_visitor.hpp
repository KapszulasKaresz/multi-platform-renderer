#ifndef IMGUI_VISITOR_HPP_INCLUDED
#define IMGUI_VISITOR_HPP_INCLUDED

#include "renderer/scene/inc/node_visitor.hpp"

namespace renderer {
namespace scene {
class ImGuiVisitor : public NodeVisitor {
public:
    ImGuiVisitor();

    virtual void visit(Node& f_node) override;
    virtual void visit(Node3D& f_node) override;
    virtual void visit(MeshInstanceNode& f_meshInstanceNode) override;

    Node* getSelectedNode() const;

    virtual ~ImGuiVisitor() = default;

private:
    Node* m_selected{ nullptr };
};

}   // namespace scene
}   // namespace renderer

#endif
