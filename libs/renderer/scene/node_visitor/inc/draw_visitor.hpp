#ifndef DRAW_VISITOR_HPP_INCLUDED
#define DRAW_VISITOR_HPP_INCLUDED

#include "renderer/scene/node_visitor/inc/node_visitor.hpp"

namespace renderer {
namespace command_buffer {
class CommandBuffer;
}   // namespace command_buffer

namespace scene {
class Camera;

class DrawVisitor : public NodeVisitor {
public:
    DrawVisitor();

    virtual void visit(Node& f_node) override;
    virtual void visit(Node3D& f_node) override;
    virtual void visit(MeshInstanceNode& f_meshInstanceNode) override;

    void setCommandBuffer(command_buffer::CommandBuffer* f_commandBuffer);
    void setCamera(Camera* f_camera);

    virtual ~DrawVisitor() = default;

protected:
    command_buffer::CommandBuffer* m_commandBuffer{ nullptr };

    Camera* m_camera{ nullptr };
};

}   // namespace scene
}   // namespace renderer

#endif
