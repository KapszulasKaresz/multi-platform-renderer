#ifndef NODE_3D_HPP_INCLUDED
#define NODE_3D_HPP_INCLUDED

#include "renderer/scene/inc/node.hpp"

namespace renderer {
namespace scene {
class Node3D : public Node {
public:
    Node3D();
    Node3D& create();

    virtual void applyVisitor(NodeVisitor* f_visitor) override;

    virtual ~Node3D() = default;

private:
};
}   // namespace scene
}   // namespace renderer

#endif
