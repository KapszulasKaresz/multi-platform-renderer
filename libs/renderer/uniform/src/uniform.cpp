#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {

Uniform& Uniform::setType(UniformType f_type)
{
    m_type = f_type;
    return *this;
}

Uniform& Uniform::setName(const std::string& f_name)
{
    m_name = f_name;
    return *this;
}

Uniform& Uniform::create()
{
    m_valid = true;
    return *this;
}

size_t Uniform::getSize() const
{
    switch (m_type) {
        case UNIFORM_TYPE_FLOAT:  return sizeof(float);
        case UNIFORM_TYPE_INT:    return sizeof(int);
        case UNIFORM_TYPE_UINT:   return sizeof(uint32_t);

        case UNIFORM_TYPE_VEC2:   return sizeof(glm::vec2);
        case UNIFORM_TYPE_IVEC2:  return sizeof(glm::ivec2);
        case UNIFORM_TYPE_UVEC2:  return sizeof(glm::uvec2);

        case UNIFORM_TYPE_VEC3:   return sizeof(glm::vec3);   // 12 bytes
        case UNIFORM_TYPE_VEC4:   return sizeof(glm::vec4);   // 16 bytes
        case UNIFORM_TYPE_IVEC3:  return sizeof(glm::ivec3);
        case UNIFORM_TYPE_IVEC4:  return sizeof(glm::ivec4);
        case UNIFORM_TYPE_UVEC3:  return sizeof(glm::uvec3);
        case UNIFORM_TYPE_UVEC4:  return sizeof(glm::uvec4);

        case UNIFORM_TYPE_MAT3X3: return sizeof(glm::mat3);   // 48 bytes
        case UNIFORM_TYPE_MAT4X4: return sizeof(glm::mat4);   // 64 bytes

        case UNIFORM_TYPE_STRUCT: return 0;   // handled by UniformCollection

        default:                  return 0;
    }
}

std::string Uniform::getName()
{
    return m_name;
}

}   // namespace uniform
}   // namespace renderer
