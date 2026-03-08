#include "renderer/scene/node/inc/node.hpp"

namespace renderer {
namespace scene {
int Node::s_nextId = 0;

Node::Node() : m_id(s_nextId++), m_name{ "Node " + std::to_string(m_id) } {}

Node& Node::create()
{
    m_valid = true;
    return *this;
}

Node& Node::setVisitorMask(uint32_t f_mask)
{
    m_visitorMask = f_mask;
    return *this;
}

Node& Node::setName(const std::string& f_name)
{
    m_name = f_name;
    return *this;
}

Node& Node::addChild(std::unique_ptr<Node>&& f_child)
{
    f_child->m_parent = this;
    m_children.push_back(std::move(f_child));
    return *this;
}

std::unique_ptr<Node> Node::removeChild(Node* f_child)
{
    auto l_it = std::find_if(
        m_children.begin(),
        m_children.end(),
        [f_child](const std::unique_ptr<Node>& f_node) {
            return *f_node.get() == *f_child;
        }
    );

    if (l_it != m_children.end()) {
        std::unique_ptr<Node> l_removedNode = std::move(*l_it);
        l_removedNode->m_parent             = nullptr;
        m_children.erase(l_it);
        return l_removedNode;
    }

    return nullptr;
}

Node* Node::getParent() const
{
    return m_parent;
}

const std::vector<std::unique_ptr<Node>>& Node::getChildren() const
{
    return m_children;
}

size_t Node::getChildCount() const
{
    return m_children.size();
}

bool Node::operator==(const Node& f_other) const
{
    return m_id == f_other.m_id;
}

int Node::getId() const
{
    return m_id;
}

const std::string& Node::getName() const
{
    return m_name;
}

void Node::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

Node::~Node() {}

}   // namespace scene
}   // namespace renderer
