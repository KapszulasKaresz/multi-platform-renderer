#include "renderer/scene/node/inc/gltf_node.hpp"

#include <iostream>

namespace renderer {
namespace scene {
GltfNode::GltfNode() : Node3D()
{
    m_name = "GLTF Node " + std::to_string(m_id);
}

GltfNode& GltfNode::create()
{
    m_valid = true;
    return *this;
}

void GltfNode::loadFromFile(const std::filesystem::path& f_filePath)
{
    tinygltf::TinyGLTF l_loader;
    tinygltf::Model    l_model;
    std::string        l_err;
    std::string        l_warn;

    bool l_ret =
        l_loader.LoadASCIIFromFile(&l_model, &l_err, &l_warn, f_filePath.string());

    if (l_warn.empty() == false) {
        std::cout << "GLTF Loader warning: " << l_warn << std::endl;
    }
}

void GltfNode::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

}   // namespace scene
}   // namespace renderer
