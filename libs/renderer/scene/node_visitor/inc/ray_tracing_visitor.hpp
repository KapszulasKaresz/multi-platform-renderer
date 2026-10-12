#ifndef RAY_TRACING_VISITOR_HPP_INCLUDED
#define RAY_TRACING_VISITOR_HPP_INCLUDED

#include "renderer/scene/node_visitor/inc/node_visitor.hpp"

namespace renderer {
namespace scene {

class RayTracingVisitor : public NodeVisitor {
public:
    RayTracingVisitor();

    virtual ~RayTracingVisitor() = default;

protected:
};

}   // namespace scene
}   // namespace renderer

#endif
