#include "renderer/scene/node/inc/gltf_node.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer/material/inc/material.hpp"
#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/uniform/inc/uniform_array.hpp"

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

    m_name = "GLTF Node " + f_filePath.filename().string();

    createDefaultMaterial();

    m_children.clear();

    bool l_ret =
        l_loader.LoadASCIIFromFile(&l_model, &l_err, &l_warn, f_filePath.string());

    if (!l_warn.empty()) {
        std::cout << "GLTF Loader warning: " << l_warn << std::endl;
    }

    if (!l_err.empty()) {
        std::cerr << "GLTF Loader error: " << l_err << std::endl;
    }
    // TODO: do all scenes
    auto l_scene = l_model.scenes[l_model.defaultScene];

    for (const auto& l_nodeIndex : l_scene.nodes) {
        loadNodesRecursively(l_model, l_nodeIndex, this);
    }
}

void GltfNode::loadNodesRecursively(
    const tinygltf::Model& f_model,
    int                    f_nodeIndex,
    Node*                  f_parent
)
{
    const auto& l_node = f_model.nodes[f_nodeIndex];

    auto l_childNode = std::make_unique<Node3D>();
    l_childNode->setName(l_node.name).create();

    if (l_node.matrix.size() == 16) {
        glm::mat4     l_transform(1.0f);
        const double* matrixData = l_node.matrix.data();
        for (int i = 0; i < 16; ++i) {
            l_transform[i / 4][i % 4] = static_cast<float>(matrixData[i]);
        }
        l_childNode->setTransform(l_transform);
    }
    else {
        glm::vec3 l_translation{ 0.0f, 0.0f, 0.0f };
        glm::quat l_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 l_scale{ 1.0f, 1.0f, 1.0f };

        if (l_node.translation.size() == 3) {
            l_translation = glm::vec3(
                static_cast<float>(l_node.translation[0]),
                static_cast<float>(l_node.translation[1]),
                static_cast<float>(l_node.translation[2])
            );
        }

        if (l_node.rotation.size() == 4) {
            l_rotation = glm::quat(
                static_cast<float>(l_node.rotation[3]),   // W
                static_cast<float>(l_node.rotation[0]),   // X
                static_cast<float>(l_node.rotation[1]),   // Y
                static_cast<float>(l_node.rotation[2])    // Z
            );
        }

        if (l_node.scale.size() == 3) {
            l_scale = glm::vec3(
                static_cast<float>(l_node.scale[0]),
                static_cast<float>(l_node.scale[1]),
                static_cast<float>(l_node.scale[2])
            );
        }

        l_childNode->setTransform(l_translation, l_rotation, l_scale);
    }

    if (l_node.mesh >= 0) {
        const auto& l_mesh = f_model.meshes[l_node.mesh];

        for (int index = 0; const auto& l_primitive : l_mesh.primitives) {
            auto l_meshInstanceNode = std::make_unique<MeshInstanceNode>();
            l_meshInstanceNode
                ->setName(l_mesh.name + " - Primitive " + std::to_string(index))
                .create();

            auto l_indices      = extractIndices(f_model, l_primitive);
            auto l_vertices     = extractVertices(f_model, l_primitive);
            auto l_rendererMesh = rendering_server::RenderingServer::getInstance()
                                      .getMainRenderingDevice()
                                      ->createTriangleMesh();
            l_rendererMesh->setIndices(l_indices).setVertices(l_vertices).create();
            l_meshInstanceNode->setMesh(l_rendererMesh)
                .setMaterial(createMaterial(f_model, l_primitive.material));

            l_childNode->addChild(std::move(l_meshInstanceNode));
            index++;
        }
    }

    f_parent->addChild(std::move(l_childNode));

    for (const auto& l_childNodeIndex : l_node.children) {
        loadNodesRecursively(
            f_model, l_childNodeIndex, f_parent->getChildren().back().get()
        );
    }
}

std::vector<uint32_t> GltfNode::extractIndices(
    const tinygltf::Model&     f_model,
    const tinygltf::Primitive& f_primitive
)
{
    std::vector<uint32_t> l_indices;

    if (f_primitive.indices < 0) {
        return l_indices;
    }

    const tinygltf::Accessor&   l_accessor   = f_model.accessors[f_primitive.indices];
    const tinygltf::BufferView& l_bufferView = f_model.bufferViews[l_accessor.bufferView];
    const tinygltf::Buffer&     l_buffer     = f_model.buffers[l_bufferView.buffer];

    l_indices.reserve(l_accessor.count);

    const uint8_t* l_dataPtr = l_buffer.data.data() + l_bufferView.byteOffset
                             + l_accessor.byteOffset;

    size_t l_byteStride = l_bufferView.byteStride;
    if (l_byteStride == 0) {
        l_byteStride = tinygltf::GetComponentSizeInBytes(l_accessor.componentType);
    }

    for (size_t i = 0; i < l_accessor.count; ++i) {
        uint32_t l_index = 0;

        switch (l_accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                l_index = *reinterpret_cast<const uint8_t*>(l_dataPtr + i * l_byteStride);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                l_index = *reinterpret_cast<const uint16_t*>(l_dataPtr + i * l_byteStride);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                l_index = *reinterpret_cast<const uint32_t*>(l_dataPtr + i * l_byteStride);
                break;
            }
            default:
                throw std::runtime_error(
                    "GltfNode::extractIndices(...) Unsupported glTF index component type."
                );
        }

        l_indices.push_back(l_index);
    }

    return l_indices;
}

std::vector<mesh::Vertex> GltfNode::extractVertices(
    const tinygltf::Model&     f_model,
    const tinygltf::Primitive& f_primitive
)
{
    std::vector<mesh::Vertex> l_vertices;
    auto                      l_posIt = f_primitive.attributes.find("POSITION");
    if (l_posIt == f_primitive.attributes.end()) {
        return l_vertices;
    }

    const tinygltf::Accessor& l_posAccessor = f_model.accessors[l_posIt->second];
    size_t                    l_vertexCount = l_posAccessor.count;

    l_vertices.resize(l_vertexCount);

    auto getAttributeData = [&](const std::string& f_name,
                                const uint8_t*&    f_outDataPtr,
                                size_t&            f_outStride) -> bool {
        auto l_it = f_primitive.attributes.find(f_name);
        if (l_it == f_primitive.attributes.end()) {
            return false;
        }

        const tinygltf::Accessor& l_accessor = f_model.accessors[l_it->second];

        if (l_accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
            throw std::runtime_error(
                "GltfNode::extractVertices(...) Attribute " + f_name
                + " is not of type FLOAT. Type conversion is required!"
            );
        }

        const tinygltf::BufferView& l_bufferView =
            f_model.bufferViews[l_accessor.bufferView];
        const tinygltf::Buffer& l_buffer = f_model.buffers[l_bufferView.buffer];

        f_outDataPtr = l_buffer.data.data() + l_bufferView.byteOffset
                     + l_accessor.byteOffset;

        if (l_bufferView.byteStride == 0) {
            f_outStride = tinygltf::GetComponentSizeInBytes(l_accessor.componentType)
                        * tinygltf::GetNumComponentsInType(l_accessor.type);
        }
        else {
            f_outStride = l_bufferView.byteStride;
        }
        return true;
    };

    const uint8_t* l_posData     = nullptr;
    size_t         l_posStride   = 0;
    const uint8_t* l_normData    = nullptr;
    size_t         l_normStride  = 0;
    const uint8_t* l_texData     = nullptr;
    size_t         l_texStride   = 0;
    const uint8_t* l_colorData   = nullptr;
    size_t         l_colorStride = 0;

    bool hasPos   = getAttributeData("POSITION", l_posData, l_posStride);
    bool hasNorm  = getAttributeData("NORMAL", l_normData, l_normStride);
    bool hasTex   = getAttributeData("TEXCOORD_0", l_texData, l_texStride);
    bool hasColor = getAttributeData("COLOR_0", l_colorData, l_colorStride);

    for (size_t i = 0; i < l_vertexCount; ++i) {
        mesh::Vertex& l_vertex = l_vertices[i];

        l_vertex.m_position = glm::vec3(0.0f);
        l_vertex.m_normal   = glm::vec3(0.0f, 1.0f, 0.0f);
        l_vertex.m_texCoord = glm::vec2(0.0f);
        l_vertex.m_color    = glm::vec3(1.0f);

        if (hasPos) {
            const float* pos = reinterpret_cast<const float*>(l_posData + i * l_posStride);
            l_vertex.m_position = glm::vec3(pos[0], pos[1], pos[2]);
        }
        if (hasNorm) {
            const float* norm =
                reinterpret_cast<const float*>(l_normData + i * l_normStride);
            l_vertex.m_normal = glm::vec3(norm[0], norm[1], norm[2]);
        }
        if (hasTex) {
            const float* tex = reinterpret_cast<const float*>(l_texData + i * l_texStride);
            l_vertex.m_texCoord = glm::vec2(tex[0], tex[1]);
        }
        if (hasColor) {
            // Note: glTF allows COLOR_0 to be VEC3 or VEC4.
            // By casting to float array, we just grab the first 3 (RGB) and ignore Alpha
            // if it's a VEC4.
            const float* l_col =
                reinterpret_cast<const float*>(l_colorData + i * l_colorStride);
            l_vertex.m_color = glm::vec3(l_col[0], l_col[1], l_col[2]);
        }
    }

    return l_vertices;
}

void GltfNode::createDefaultMaterial()
{
    m_defaultMaterial = rendering_server::RenderingServer::getInstance()
                            .getMainRenderingDevice()
                            ->createMaterial();

    auto l_image = rendering_server::RenderingServer::getInstance()
                       .getMainRenderingDevice()
                       ->createImage();

    l_image->generateMipMaps()
        .setSize(glm::ivec2(1, 1))
        .setFormat(image::ImageFormat::IMAGE_FORMAT_RGBA8_SRGB)
        .createEmptyImage();

    auto l_texture = rendering_server::RenderingServer::getInstance()
                         .getMainRenderingDevice()
                         ->createTexture();
    l_texture->setImage(l_image).create();

    auto l_uniformCollectionCamera = rendering_server::RenderingServer::getInstance()
                                         .getMainRenderingDevice()
                                         ->createUniformCollection();
    l_uniformCollectionCamera->setUnique(true);

    l_uniformCollectionCamera->addMember("view")
        ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
        .create();

    l_uniformCollectionCamera->addMember("proj")
        ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
        .create();

    l_uniformCollectionCamera->setName("Camera").create();

    auto l_uniformCollectionObject = rendering_server::RenderingServer::getInstance()
                                         .getMainRenderingDevice()
                                         ->createUniformCollection();
    l_uniformCollectionObject->setUnique(false);
    l_uniformCollectionObject->addMember("model")
        ->setType(renderer::uniform::UNIFORM_TYPE_MAT4X4)
        .create();

    l_uniformCollectionObject->addTexture(l_texture);
    l_uniformCollectionObject->setName("Object").create();

    auto l_lightArray = rendering_server::RenderingServer::getInstance()
                            .getMainRenderingDevice()
                            ->createUniformCollection();

    l_lightArray->setName("Lights");

    auto l_light0 = rendering_server::RenderingServer::getInstance()
                        .getMainRenderingDevice()
                        ->createUniformCollection();
    l_light0->addMember("position")
        ->setType(renderer::uniform::UNIFORM_TYPE_VEC4)
        .setValue(glm::vec4(0.0f, 10.0f, 0.0f, 1.0f))
        .create();

    l_light0->addMember("emission")
        ->setType(renderer::uniform::UNIFORM_TYPE_VEC4)
        .setValue(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f))
        .create();

    l_light0->setName("Light_0")
        .setType(renderer::uniform::UNIFORM_TYPE_STRUCT_MEMBER)
        .create();

    l_lightArray->addMember("lightCount")
        ->setType(renderer::uniform::UNIFORM_TYPE_INT)
        .setValue(2)
        .create();

    l_lightArray->addMember(l_light0);

    auto l_light1 = rendering_server::RenderingServer::getInstance()
                        .getMainRenderingDevice()
                        ->createUniformCollection();
    l_light1->addMember("position")
        ->setType(renderer::uniform::UNIFORM_TYPE_VEC4)
        .setValue(glm::vec4(10.0f, 10.0f, 0.0f, 1.0f))
        .create();

    l_light1->addMember("emission")
        ->setType(renderer::uniform::UNIFORM_TYPE_VEC4)
        .setValue(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f))
        .create();

    l_light1->setName("Light_1")
        .setType(renderer::uniform::UNIFORM_TYPE_STRUCT_MEMBER)
        .create();

    l_lightArray->addMember(l_light1);

    l_lightArray->create();
    m_defaultMaterial->setShader("res/shaders/shader")
        .addUniformCollection(l_uniformCollectionCamera)
        .addUniformCollection(l_uniformCollectionObject)
        .addUniformCollection(l_lightArray)
        .create();
}

std::shared_ptr<material::Material>
    GltfNode::createMaterial(const tinygltf::Model& f_model, int f_materialIndex)
{
    auto l_materialRet = rendering_server::RenderingServer::getInstance()
                             .getMainRenderingDevice()
                             ->createMaterial();
    l_materialRet->copyMaterial(m_defaultMaterial).create();

    const auto& l_material             = f_model.materials[f_materialIndex];
    const auto& l_pbrMetallicRoughness = l_material.pbrMetallicRoughness;

    auto l_image = rendering_server::RenderingServer::getInstance()
                       .getMainRenderingDevice()
                       ->createImage();

    if (l_pbrMetallicRoughness.baseColorTexture.index != -1) {
        const auto& l_texture =
            f_model.textures[l_pbrMetallicRoughness.baseColorTexture.index];
        const auto& l_imageData = f_model.images[l_texture.source];
        l_image->generateMipMaps().createFromGltfImage(l_imageData);

        // TODO sampler
    }
    else {
        l_image->generateMipMaps().createFromFile("res/textures/test_texture.png");
    }

    auto l_texture = rendering_server::RenderingServer::getInstance()
                         .getMainRenderingDevice()
                         ->createTexture();
    l_texture->setImage(l_image).create();

    auto l_uniformCollection = l_materialRet->getUniformCollection("Object");
    if (l_uniformCollection) {
        l_uniformCollection->addTexture(l_texture, 0);
    }

    return l_materialRet;
}

void GltfNode::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

std::shared_ptr<material::Material> GltfNode::getMaterial() const
{
    return m_defaultMaterial;
}

}   // namespace scene
}   // namespace renderer
