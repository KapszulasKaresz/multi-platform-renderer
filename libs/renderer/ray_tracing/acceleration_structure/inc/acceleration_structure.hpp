#ifndef ACCELERATION_STRUCTURE_HPP_INCLUDED
#define ACCELERATION_STRUCTURE_HPP_INCLUDED

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace scene {
class Node;
}

namespace ray_tracing {
class AccelerationStructure : public RenderResource {
public:
    virtual AccelerationStructure& setScene(scene::Node* f_rootNode) = 0;
    virtual AccelerationStructure& create() = 0;
};
}   // namespace ray_tracing
}   // namespace renderer

#endif
