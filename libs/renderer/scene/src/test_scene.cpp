#include "renderer/scene/inc/test_scene.hpp"

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
#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"
#include "renderer/uniform/inc/uniform_single.hpp"
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
    m_rootNode->addChild(std::move(l_gltfNode));

    auto* l_renderingDevice =
        rendering_server::RenderingServer::getInstance().getMainRenderingDevice();
    m_accelerationStructure = l_renderingDevice->createAccelerationStructure();
    if (m_accelerationStructure) {
        m_accelerationStructure->setScene(m_rootNode.get()).create();
        m_rayTracingMaterial      = l_renderingDevice->createMaterial();
        m_rayTracingRenderTarget = l_renderingDevice->createRenderTarget();
        m_rayTracingRenderTarget->setFormat(image::IMAGE_FORMAT_BGRA8)
            .setColorSpace(image::COLOR_SPACE_LINEAR)
            .setSize({ 1920, 1080 })
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
        m_rayTracingCameraUniform->setName("Camera").create();

        auto l_accelerationStructureUniform =
            l_renderingDevice->createUniformAccelerationStructure();
        l_accelerationStructureUniform->setName("SceneAccelerationStructure");
        l_accelerationStructureUniform->setAccelerationStructure(m_accelerationStructure);
        l_accelerationStructureUniform->create();
        m_rayTracingMaterial
            ->setMaterialType(material::MaterialType::MATERIAL_TYPE_RAY_TRACING)
            .setShader("res/shaders/compiled/raytracing")
            .addUniformCollection(m_rayTracingCameraUniform)
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
    }

    f_commandBuffer->useViewport({ .m_fullScreen = true });

    ImGui::Begin("Rendering");
    ImGui::Checkbox("Use ray tracing", &m_useRayTracing);
    ImGui::End();

    if (!isRayTracingEnabled()) {
        if (!isRayTracingEnabled()) {
            m_drawVisitor.setCommandBuffer(f_commandBuffer);
            m_rootNode->applyVisitor(&m_drawVisitor);
        }
    }

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
    render_target::RenderTarget*    f_renderTarget,
    uint32_t                       f_width,
    uint32_t                       f_height
)
{
    f_commandBuffer->traceRays(
        m_rayTracingMaterial, m_rayTracingRenderTarget.get(), 1920, 1080
    );
    f_commandBuffer->copyRenderTarget(m_rayTracingRenderTarget.get(), f_renderTarget);
}
}   // namespace scene
}   // namespace renderer
