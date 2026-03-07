#ifndef IMGUI_VISITOR_HPP_INCLUDED
#define IMGUI_VISITOR_HPP_INCLUDED

#include "renderer/scene/inc/node_visitor.hpp"

namespace renderer {
namespace scene {
class ImGuiVisitor : public NodeVisitor {
public:
    ImGuiVisitor();
    virtual void visit(Node& f_node) override;
};

}   // namespace scene
}   // namespace renderer

#endif
