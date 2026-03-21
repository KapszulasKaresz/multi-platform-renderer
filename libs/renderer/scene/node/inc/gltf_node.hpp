#ifndef GLTF_NODE_HPP_INCLUDED
#define GLTF_NODE_HPP_INCLUDED

#include <filesystem>

#include <tiny_gltf.h>

#include "renderer/scene/node/inc/node_3d.hpp"

namespace renderer {
namespace scene {

class GltfNode : public Node3D {
public:
    GltfNode();
    GltfNode& create();

    void loadFromFile(const std::filesystem::path& f_filePath);

    virtual void applyVisitor(NodeVisitor* f_visitor) override;

    virtual ~GltfNode() = default;

protected:
};

}   // namespace scene
}   // namespace renderer

#endif
