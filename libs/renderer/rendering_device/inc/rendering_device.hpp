#ifndef RENDERING_DEVICE_HPP_INCLUDED
#define RENDERING_DEVICE_HPP_INCLUDED

#include <memory>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace uniform {
class UniformCollection;
}   // namespace uniform

namespace command_buffer {
class CommandBuffer;
}   // namespace command_buffer

namespace material {
class Material;
}   // namespace material

namespace mesh {

class TriangleMesh;
}   // namespace mesh

namespace image {
class Image;
}   // namespace image

namespace render_target {
class RenderTargetWindow;
}   // namespace render_target

namespace window {
class Window;
}   // namespace window

namespace rendering_device {
class RenderingDevice : public RenderResource {
public:
    virtual std::shared_ptr<render_target::RenderTargetWindow> getRenderTargetWindow() = 0;

    virtual RenderingDevice& setWindow(window::Window* f_window) = 0;

    virtual std::shared_ptr<image::Image>                  createImage()         = 0;
    virtual std::shared_ptr<material::Material>            createMaterial()      = 0;
    virtual std::shared_ptr<command_buffer::CommandBuffer> createCommandBuffer() = 0;
    virtual std::shared_ptr<command_buffer::CommandBuffer> getRenderingCommandBuffer() = 0;
    virtual std::shared_ptr<mesh::TriangleMesh>         createTriangleMesh()      = 0;
    virtual std::shared_ptr<uniform::UniformCollection> createUniformCollection() = 0;

    virtual bool preFrame()        = 0;
    virtual void postFrame()       = 0;
    virtual void finishRendering() = 0;

    uint32_t getCurrentFrame() const;
    int      getMaxFramesInFlight() const;

    virtual bool isValid();

protected:
    int      m_maxFramesInFlight{ 2 };
    uint32_t m_currentFrame{ 0 };
};

}   // namespace rendering_device
}   // namespace renderer

#endif
