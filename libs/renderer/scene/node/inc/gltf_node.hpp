#ifndef GLTF_NODE_HPP_INCLUDED
#define GLTF_NODE_HPP_INCLUDED

#include <filesystem>
#include <vector>

#include <tiny_gltf.h>

#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/scene/node/inc/node_3d.hpp"

namespace renderer {
namespace material {
class Material;
}   // namespace material

namespace scene {

class GltfNode : public Node3D {
public:
    GltfNode();
    GltfNode& create();
    void      loadFromFile(const std::filesystem::path& f_filePath);

    virtual void applyVisitor(NodeVisitor* f_visitor) override;

    virtual ~GltfNode() = default;

protected:
    void loadNodesRecursively(
        const tinygltf::Model& f_model,
        int                    f_nodeIndex,
        Node*                  f_parent
    );

    std::vector<uint32_t> extractIndices(
        const tinygltf::Model&     f_model,
        const tinygltf::Primitive& f_primitive
    );

    std::vector<mesh::Vertex> extractVertices(
        const tinygltf::Model&     f_model,
        const tinygltf::Primitive& f_primitive
    );

    std::shared_ptr<material::Material>
        createMaterial(const tinygltf::Model& f_model, int f_materialIndex);
};

}   // namespace scene
}   // namespace renderer

#endif
