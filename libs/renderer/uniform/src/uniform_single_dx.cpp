#include "renderer/uniform/inc/uniform_single_dx.hpp"

namespace renderer {
namespace uniform {
size_t UniformSingleDX::getAlignment() const
{
    switch (m_type) {
        case UNIFORM_TYPE_FLOAT:  return alignof(float);
        case UNIFORM_TYPE_INT:    return alignof(int);
        case UNIFORM_TYPE_UINT:   return alignof(uint32_t);

        case UNIFORM_TYPE_VEC2:   return alignof(glm::vec2);
        case UNIFORM_TYPE_IVEC2:  return alignof(glm::ivec2);
        case UNIFORM_TYPE_UVEC2:  return alignof(glm::uvec2);

        case UNIFORM_TYPE_VEC3:   return alignof(glm::vec4);   // vec3 padded like vec4
        case UNIFORM_TYPE_VEC4:   return alignof(glm::vec4);
        case UNIFORM_TYPE_IVEC3:  return alignof(glm::ivec4);
        case UNIFORM_TYPE_IVEC4:  return alignof(glm::ivec4);
        case UNIFORM_TYPE_UVEC3:  return alignof(glm::uvec4);
        case UNIFORM_TYPE_UVEC4:  return alignof(glm::uvec4);

        case UNIFORM_TYPE_MAT3X3: return alignof(glm::mat4);   // treated as array of vec4
        case UNIFORM_TYPE_MAT4X4: return alignof(glm::mat4);

        case UNIFORM_TYPE_STRUCT: return 16;   // minimum struct alignment

        default:                  return 0;
    }
}
}   // namespace uniform
}   // namespace renderer
