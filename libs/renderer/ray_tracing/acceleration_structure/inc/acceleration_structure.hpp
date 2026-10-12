#ifndef ACCELERATION_STRUCTURE_HPP_INCLUDED
#define ACCELERATION_STRUCTURE_HPP_INCLUDED

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace ray_tracing {
class AccelerationStructure : public RenderResource {
public:
    virtual AccelerationStructure& create() = 0;
};
}   // namespace ray_tracing
}   // namespace renderer

#endif
