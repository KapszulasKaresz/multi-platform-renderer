#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace material {
class Material : public RenderResource {
public:
    virtual Material& create() = 0;

protected:
};
}   // namespace material
}   // namespace  renderer


#endif
