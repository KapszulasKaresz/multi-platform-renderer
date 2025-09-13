#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace material {
Material& Material::setShader(std::string f_shaderPath)
{
    m_shaderLocation = f_shaderPath;
    return *this;
}

}   // namespace material
}   // namespace renderer
