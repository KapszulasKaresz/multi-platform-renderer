#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/scene/inc/node_visitor.hpp"

namespace renderer {
namespace scene {

class Node : public RenderResource {
public:
    Node();

    Node& create();
    Node& setVisitorMask(uint32_t f_mask);
    Node& setName(const std::string& f_name);

    Node&                                     addChild(std::unique_ptr<Node>&& f_child);
    std::unique_ptr<Node>                     removeChild(Node* f_child);
    const std::vector<std::unique_ptr<Node>>& getChildren() const;
    size_t                                    getChildCount() const;

    Node* getParent() const;
    bool  operator==(const Node& f_other) const;

    int                getId() const;
    const std::string& getName() const;

    virtual void applyVisitor(NodeVisitor* f_visitor);

protected:
    Node*                              m_parent{ nullptr };
    std::vector<std::unique_ptr<Node>> m_children;

    int         m_id{ -1 };
    std::string m_name{};

    uint32_t m_visitorMask{ VISITOR_MASK_ALL };

    static int s_nextId;
};
}   // namespace scene
}   // namespace renderer

#endif
