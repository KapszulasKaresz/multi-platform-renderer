#include "renderer/scene/node_visitor/inc/draw_visitor.hpp"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/scene/camera/inc/camera.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node/inc/node.hpp"
#include "renderer/utils/inc/imgui_functions.hpp"

namespace renderer {
namespace scene {
DrawVisitor::DrawVisitor() : NodeVisitor()
{
    m_mask = VISITOR_MASK_DRAW;
}

void DrawVisitor::visit(Node& f_node)
{
    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void DrawVisitor::visit(Node3D& f_node)
{
    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void DrawVisitor::visit(MeshInstanceNode& f_node)
{
    if (not m_commandBuffer && not m_camera) {
        return;
    }

    if (f_node.getMesh() && f_node.getMaterial()) {
        auto l_uniformCollection = f_node.getMaterial()->getUniformCollection("Camera");

        uniform::UniformSingle* l_modelUniform = dynamic_cast<uniform::UniformSingle*>(
            l_uniformCollection->getMember("model")
        );

        l_modelUniform->setValue(f_node.M());

        uniform::UniformSingle* l_viewUniform =
            dynamic_cast<uniform::UniformSingle*>(l_uniformCollection->getMember("view"));

        l_viewUniform->setValue(m_camera->V());

        uniform::UniformSingle* l_projectionUniform =
            dynamic_cast<uniform::UniformSingle*>(l_uniformCollection->getMember("proj"));

        l_projectionUniform->setValue(m_camera->P());

        m_commandBuffer->useMaterial(f_node.getMaterial());
        m_commandBuffer->useViewport({ .m_fullScreen = true });
        m_commandBuffer->draw(f_node.getMesh());
    }

    for (const auto& l_child : f_node.getChildren()) {
        l_child->applyVisitor(this);
    }
}

void DrawVisitor::setCommandBuffer(command_buffer::CommandBuffer* f_commandBuffer)
{
    m_commandBuffer = f_commandBuffer;
}

void DrawVisitor::setCamera(Camera* f_camera)
{
    m_camera = f_camera;
}

}   // namespace scene
}   // namespace renderer
