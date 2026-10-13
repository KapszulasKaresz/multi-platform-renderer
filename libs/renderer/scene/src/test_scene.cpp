#include "renderer/scene/inc/test_scene.hpp"

#include <cstring>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/material/inc/material.hpp"
#include "renderer/render_target/inc/render_target.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/scene/node/inc/gltf_node.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node_visitor/inc/imgui_visitor.hpp"
#include "renderer/scene/node_visitor/inc/ray_tracing_visitor.hpp"
#include "renderer/texture/inc/texture.hpp"
#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"
#include "renderer/uniform/inc/uniform_single.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer.hpp"
#include "renderer/utils/inc/imgui_functions.hpp"

namespace renderer {
namespace scene {

TestScene::TestScene() = default;

TestScene::~TestScene() = default;

TestScene& TestScene::setMesh(std::shared_ptr<mesh::TriangleMesh> f_mesh)
{
    m_mesh = f_mesh;
    return *this;
}

TestScene& TestScene::create()
{
    m_rootNode = std::make_unique<Node>();
    m_rootNode->setName("Root Node").create();

    auto l_gltfNode = std::make_unique<GltfNode>();
    l_gltfNode->setName("GLTF Node").create();
    // l_gltfNode->loadFromFile("res/models/glTF-Sample-Models/2.0/Buggy/glTF/Buggy.gltf");
    l_gltfNode->loadFromFile("res/models/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
    // l_gltfNode->loadFromFile(
    //     "res/models/glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf"
    // );
    m_rootNode->addChild(std::move(l_gltfNode));

    auto* l_renderingDevice =
        rendering_server::RenderingServer::getInstance().getMainRenderingDevice();
    m_accelerationStructure = l_renderingDevice->createAccelerationStructure();
    if (m_accelerationStructure) {
        m_accelerationStructure->setScene(m_rootNode.get()).create();
        RayTracingVisitor l_materialVisitor;
        m_rootNode->applyVisitor(&l_materialVisitor);

        struct MeshHeader {
            uint32_t vertexOffset;
            uint32_t indexOffset;
            uint32_t indexCount;
            uint32_t reserved;
        };

        struct GeometryHeader {
            uint32_t meshCount;
            uint32_t vertexCount;
        };

        std::vector<std::byte> l_geometryData(sizeof(GeometryHeader));
        std::unordered_map<mesh::TriangleMesh*, uint32_t> l_meshOffsets;
        std::vector<MeshHeader>                           l_headers;
        std::vector<glm::vec2>                            l_uvs;
        std::vector<uint32_t>                             l_indices;
        for (const auto& l_instance : l_materialVisitor.getMeshInstances()) {
            if (l_meshOffsets.contains(l_instance.m_mesh.get())) {
                continue;
            }
            const uint32_t l_meshId = static_cast<uint32_t>(l_headers.size());
            l_meshOffsets.emplace(l_instance.m_mesh.get(), l_meshId);
            const auto& l_meshUvs     = l_instance.m_mesh->getVertices();
            const auto& l_meshIndices = l_instance.m_mesh->getIndices();
            l_headers.push_back(
                { .vertexOffset = static_cast<uint32_t>(l_uvs.size()),
                  .indexOffset  = static_cast<uint32_t>(l_indices.size()),
                  .indexCount   = static_cast<uint32_t>(l_meshIndices.size()) }
            );
            for (const auto& l_vertex : l_meshUvs) {
                l_uvs.push_back(l_vertex.m_texCoord);
            }
            l_indices.insert(l_indices.end(), l_meshIndices.begin(), l_meshIndices.end());
        }
        const uint32_t l_headerBytes = static_cast<uint32_t>(
            sizeof(GeometryHeader) + l_headers.size() * sizeof(MeshHeader)
        );
        l_geometryData.resize(
            l_headerBytes + l_uvs.size() * sizeof(glm::vec2)
            + l_indices.size() * sizeof(uint32_t)
        );
        GeometryHeader l_geometryHeader{ static_cast<uint32_t>(l_headers.size()),
                                         static_cast<uint32_t>(l_uvs.size()) };
        std::memcpy(l_geometryData.data(), &l_geometryHeader, sizeof(l_geometryHeader));
        std::memcpy(
            l_geometryData.data() + sizeof(l_geometryHeader),
            l_headers.data(),
            l_headers.size() * sizeof(MeshHeader)
        );
        std::memcpy(
            l_geometryData.data() + l_headerBytes,
            l_uvs.data(),
            l_uvs.size() * sizeof(glm::vec2)
        );
        std::memcpy(
            l_geometryData.data() + l_headerBytes + l_uvs.size() * sizeof(glm::vec2),
            l_indices.data(),
            l_indices.size() * sizeof(uint32_t)
        );
        auto l_geometryBuffer = l_renderingDevice->createUniformStorageBuffer();
        l_geometryBuffer->create();
        l_geometryBuffer->setValueRaw(l_geometryData);
        auto l_lightArray = l_renderingDevice->createUniformCollection();
        l_lightArray->setName("Lights");
        auto createLight = [&](const glm::vec4&   f_position,
                               const glm::vec4&   f_emission,
                               const std::string& f_name) {
            auto l_light = l_renderingDevice->createUniformCollection();
            l_light->addMember("position")
                ->setType(uniform::UNIFORM_TYPE_VEC4)
                .setValue(f_position)
                .create();
            l_light->addMember("emission")
                ->setType(uniform::UNIFORM_TYPE_VEC4)
                .setValue(f_emission)
                .create();
            l_light->setName(f_name).setType(uniform::UNIFORM_TYPE_STRUCT_MEMBER).create();
            l_lightArray->addMember(l_light);
        };
        l_lightArray->addMember("lightCount")
            ->setType(uniform::UNIFORM_TYPE_INT)
            .setValue(2)
            .create();
        createLight({ 0.0f, 10.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, "Light_0");
        createLight({ 10.0f, 10.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, "Light_1");
        l_lightArray->addMember("sunlightDirection")
            ->setType(uniform::UNIFORM_TYPE_VEC4)
            .setValue(glm::vec4(-0.45f, -1.0f, -0.35f, 0.0f))
            .create();
        l_lightArray->addMember("sunlightEmission")
            ->setType(uniform::UNIFORM_TYPE_VEC4)
            .setValue(glm::vec4(150.0f, 150.0f, 150.0f, 1.0f))
            .create();
        l_lightArray->create();
        std::vector<std::shared_ptr<texture::Texture>> l_albedoTextures;
        std::vector<std::shared_ptr<texture::Texture>> l_normalTextures;
        std::vector<std::shared_ptr<texture::Texture>> l_metallicRoughnessTextures;
        std::unordered_set<material::Material*>        l_seenMaterials;
        for (const auto& l_instance : l_materialVisitor.getMeshInstances()) {
            if (l_instance.m_materialId >= 128) {
                throw std::runtime_error(
                    "TestScene::create() ray-tracing material count exceeds 128"
                );
            }
            if (!l_instance.m_material
                || !l_seenMaterials.insert(l_instance.m_material.get()).second)
            {
                continue;
            }
            const size_t l_requiredSize = l_instance.m_materialId + 1;
            l_albedoTextures.resize(l_requiredSize);
            l_normalTextures.resize(l_requiredSize);
            l_metallicRoughnessTextures.resize(l_requiredSize);
            auto l_objectUniform = l_instance.m_material->getUniformCollection("Object");
            if (!l_objectUniform || l_objectUniform->getTextureCount() < 3) {
                throw std::runtime_error(
                    "TestScene::create() material is missing PBR textures"
                );
            }
            l_albedoTextures[l_instance.m_materialId] =
                l_objectUniform->getTextureShared(0);
            l_metallicRoughnessTextures[l_instance.m_materialId] =
                l_objectUniform->getTextureShared(1);
            l_normalTextures[l_instance.m_materialId] =
                l_objectUniform->getTextureShared(2);
            if (!l_albedoTextures[l_instance.m_materialId]
                || !l_normalTextures[l_instance.m_materialId]
                || !l_metallicRoughnessTextures[l_instance.m_materialId])
            {
                throw std::runtime_error(
                    "TestScene::create() ray-tracing material texture table has a gap"
                );
            }
        }
        for (size_t l_materialId = 0; l_materialId < l_albedoTextures.size();
             ++l_materialId)
        {
            if (!l_albedoTextures[l_materialId]
                || !l_normalTextures[l_materialId]
                || !l_metallicRoughnessTextures[l_materialId])
            {
                throw std::runtime_error(
                    "TestScene::create() ray-tracing material texture table has a gap"
                );
            }
        }
        m_rayTracingMaterial     = l_renderingDevice->createMaterial();
        m_rayTracingRenderTarget = l_renderingDevice->createRenderTarget();
        m_rayTracingRenderTarget->setFormat(image::IMAGE_FORMAT_BGRA8)
            .setColorSpace(image::COLOR_SPACE_LINEAR)
            .setSize({ 1'920, 1'080 })
            .setStorageImage()
            .create();
        m_rayTracingCameraUniform = l_renderingDevice->createUniformCollection();
        m_rayTracingCameraUniform->setUnique(true);
        m_rayTracingCameraUniform->addMember("view")
            ->setType(uniform::UNIFORM_TYPE_MAT4X4)
            .create();
        m_rayTracingCameraUniform->addMember("proj")
            ->setType(uniform::UNIFORM_TYPE_MAT4X4)
            .create();
        m_rayTracingCameraUniform->addMember("inverseView")
            ->setType(uniform::UNIFORM_TYPE_MAT4X4)
            .create();
        m_rayTracingCameraUniform->addMember("inverseProj")
            ->setType(uniform::UNIFORM_TYPE_MAT4X4)
            .create();
        m_rayTracingCameraUniform->addMember("accumulationFrame")
            ->setType(uniform::UNIFORM_TYPE_UINT)
            .setValue(0u)
            .create();
        m_rayTracingCameraUniform->setName("Camera").create();

        auto l_accelerationStructureUniform =
            l_renderingDevice->createUniformAccelerationStructure();
        l_accelerationStructureUniform->setName("SceneAccelerationStructure");
        l_accelerationStructureUniform->setAccelerationStructure(m_accelerationStructure);
        l_accelerationStructureUniform->create();
        m_rayTracingMaterial
            ->setMaterialType(material::MaterialType::MATERIAL_TYPE_RAY_TRACING)
            .setRayTracingAlbedoTextures(std::move(l_albedoTextures))
            .setRayTracingNormalTextures(std::move(l_normalTextures))
            .setRayTracingMetallicRoughnessTextures(std::move(l_metallicRoughnessTextures))
            .setShader("res/shaders/compiled/raytracing")
            .addUniformCollection(m_rayTracingCameraUniform)
            .addUniformCollection(l_lightArray)
            .addUniformStorageBuffer(l_geometryBuffer)
            .addUniformAccelerationStructure(l_accelerationStructureUniform)
            .create();
    }

    m_camera = std::make_unique<PerspectiveCamera>();
    m_drawVisitor.setCamera(m_camera.get());

    m_observer = std::make_unique<ControlledObserver>();
    m_observer->setWindow(rendering_server::RenderingServer::getInstance().getWindow());
    m_observer->setCamera(m_camera.get());

    m_valid = true;
    return *this;
}

void TestScene::recordCommandBuffer(
    command_buffer::CommandBuffer* f_commandBuffer,
    float                          f_deltaTime
)
{
    m_camera->setAspectRatio(
        static_cast<float>(f_commandBuffer->getCurrentRenderTarget()->getSize().x)
        / static_cast<float>(f_commandBuffer->getCurrentRenderTarget()->getSize().y)
    );

    m_observer->update(f_deltaTime);
    const glm::mat4 l_currentView   = m_camera->V();
    bool            l_cameraChanged = false;
    for (int i = 0; i < 4 && !l_cameraChanged; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (std::abs(l_currentView[i][j] - m_previousCameraView[i][j]) > 0.0001f) {
                l_cameraChanged = true;
                break;
            }
        }
    }
    if (!m_wasRayTracingEnabled || l_cameraChanged) {
        m_rayTracingFrame = 0;
    }
    m_previousCameraView = l_currentView;
    if (m_rayTracingCameraUniform) {
        auto* l_view = dynamic_cast<uniform::UniformSingle*>(
            m_rayTracingCameraUniform->getMember("view")
        );
        auto* l_projection = dynamic_cast<uniform::UniformSingle*>(
            m_rayTracingCameraUniform->getMember("proj")
        );
        auto* l_inverseView = dynamic_cast<uniform::UniformSingle*>(
            m_rayTracingCameraUniform->getMember("inverseView")
        );
        auto* l_inverseProjection = dynamic_cast<uniform::UniformSingle*>(
            m_rayTracingCameraUniform->getMember("inverseProj")
        );
        l_view->setValue(m_camera->V());
        l_projection->setValue(m_camera->P());
        l_inverseView->setValue(glm::inverse(m_camera->V()));
        l_inverseProjection->setValue(glm::inverse(m_camera->P()));
        dynamic_cast<uniform::UniformSingle*>(
            m_rayTracingCameraUniform->getMember("accumulationFrame")
        )
            ->setValue(m_rayTracingFrame);
    }

    f_commandBuffer->useViewport({ .m_fullScreen = true });

    ImGui::Begin("Rendering");
    ImGui::Checkbox("Use ray tracing", &m_useRayTracing);
    ImGui::End();

    if (!isRayTracingEnabled()) {
        m_drawVisitor.setCommandBuffer(f_commandBuffer);
        m_rootNode->applyVisitor(&m_drawVisitor);
    }
    m_wasRayTracingEnabled = isRayTracingEnabled();

    ImGui::Begin("Test Scene tree");
    m_rootNode->applyVisitor(&m_imguiVisitor);
    ImGui::End();

    ImGui::Begin("Test Scene selected node");
    auto l_selectedNode = m_imguiVisitor.getSelectedNode();
    if (l_selectedNode) {
        ImGui::DrawNodeDynamic(*l_selectedNode);
    }
    ImGui::End();
}

bool TestScene::isRayTracingEnabled() const
{
    return m_useRayTracing && m_rayTracingMaterial != nullptr;
}

void TestScene::recordRayTracingCommandBuffer(
    command_buffer::CommandBuffer* f_commandBuffer,
    render_target::RenderTarget*   f_renderTarget,
    uint32_t                       f_width,
    uint32_t                       f_height
)
{
    f_commandBuffer->traceRays(
        m_rayTracingMaterial, m_rayTracingRenderTarget.get(), 1'920, 1'080
    );
    f_commandBuffer->copyRenderTarget(m_rayTracingRenderTarget.get(), f_renderTarget);
    ++m_rayTracingFrame;
}
}   // namespace scene
}   // namespace renderer
