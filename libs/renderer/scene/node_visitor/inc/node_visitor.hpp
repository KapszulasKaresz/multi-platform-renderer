#ifndef NODE_VISITOR_HPP_INCLUDED
#define NODE_VISITOR_HPP_INCLUDED

#include <cstdint>

namespace renderer {
namespace scene {
class Node;
class Node3D;
class MeshInstanceNode;
class GltfNode;

enum VisitorMask : uint32_t {
    VISITOR_MASK_NONE = 0,
    VISITOR_MASK_DRAW = 1 << 0,
    VISITOR_MASK_UI   = 1 << 1,
    VISITOR_MASK_ALL  = ~0u,
};

class NodeVisitor {
public:
    virtual void visit(Node& f_node);
    virtual void visit(Node3D& f_node);
    virtual void visit(MeshInstanceNode& f_meshInstanceNode);
    virtual void visit(GltfNode& f_node);


    VisitorMask getMask() const;

    virtual ~NodeVisitor() = default;

protected:
    uint32_t m_mask{ VISITOR_MASK_ALL };
};

}   // namespace scene
}   // namespace renderer

#endif
