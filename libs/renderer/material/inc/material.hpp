#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include <string>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace material {
class Material : public RenderResource {
public:
    virtual Material& setShader(std::string f_shaderPath);
    virtual Material& create() = 0;

protected:
    std::string m_shaderLocation{ "" };
};
}   // namespace material
}   // namespace  renderer


#endif
