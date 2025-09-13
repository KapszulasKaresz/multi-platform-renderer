#ifndef COMMAND_BUFFER_HPP
#define COMMAND_BUFFER_HPP

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace command_buffer {
class CommandBuffer : public RenderResource {
public:
    virtual CommandBuffer& submit()         = 0;
    virtual CommandBuffer& reset()          = 0;
    virtual CommandBuffer& begin()          = 0;
    virtual CommandBuffer& end()            = 0;
    virtual CommandBuffer& beginRendering() = 0;
    virtual CommandBuffer& endRendering()   = 0;
};
}   // namespace command_buffer
}   // namespace  renderer


#endif
