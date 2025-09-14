#ifndef COMMAND_BUFFER_HPP
#define COMMAND_BUFFER_HPP

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace render_target {
class RenderTarget;
}   // namespace render_target

namespace command_buffer {
class CommandBuffer : public RenderResource {
public:
    virtual CommandBuffer& submit() = 0;
    virtual CommandBuffer& reset()  = 0;
    virtual CommandBuffer& begin()  = 0;
    virtual CommandBuffer& end()    = 0;
    virtual CommandBuffer& beginRendering(
        std::shared_ptr<render_target::RenderTarget> f_renderTarget,
        glm::vec4 f_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    )                                     = 0;
    virtual CommandBuffer& endRendering() = 0;
};
}   // namespace command_buffer
}   // namespace  renderer


#endif
