#include "renderer/scene/node_visitor/inc/draw_visitor.hpp"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/scene/camera/inc/camera.hpp"
#include "renderer/scene/node/inc/gltf_node.hpp"
#include "renderer/scene/node/inc/mesh_instance_node.hpp"
#include "renderer/scene/node/inc/node.hpp"
#include "renderer/utils/inc/imgui_functions.hpp"

namespace renderer {
namespace scene {
DrawVisitor::DrawVisitor() : NodeVisitor()
{
    m_mask = VISITOR_MASK_DRAW;
}

void DrawVisitor::visit(MeshInstanceNode& f_node)
{
    if (not m_commandBuffer && not m_camera) {
        return;
    }

    if (f_node.getMesh() && f_node.getMaterial()) {
        auto l_uniformCollectionCamera =
            f_node.getMaterial()->getUniformCollection("Camera");

        uniform::UniformSingle* l_modelUniform = dynamic_cast<uniform::UniformSingle*>(
            f_node.getMaterial()->getUniformCollection("Object")->getMember("model")
        );

        l_modelUniform->setValue(f_node.M());

        uniform::UniformSingle* l_viewUniform = dynamic_cast<uniform::UniformSingle*>(
            l_uniformCollectionCamera->getMember("view")
        );

        l_viewUniform->setValue(m_camera->V());

        uniform::UniformSingle* l_projectionUniform =
            dynamic_cast<uniform::UniformSingle*>(
                l_uniformCollectionCamera->getMember("proj")
            );

        l_projectionUniform->setValue(m_camera->P());

        if (f_node.getMaterial()->isOriginal()) {
            m_commandBuffer->useMaterial(f_node.getMaterial());
        }
        else {
            m_commandBuffer->updateUniforms(f_node.getMaterial());
        }

        m_commandBuffer->draw(f_node.getMesh());
    }

    NodeVisitor::visit(f_node);
}

void DrawVisitor::visit(GltfNode& f_node)
{
    m_commandBuffer->useMaterial(f_node.getMaterial());
    NodeVisitor::visit(f_node);
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
