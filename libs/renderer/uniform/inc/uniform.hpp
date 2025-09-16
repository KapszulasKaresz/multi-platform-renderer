#ifndef UNIFORM_HPP_INCLUDED
#define UNIFORM_HPP_INCLUDED

#include <string>
#include <variant>

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace uniform {

enum UniformType {
    UNIFORM_TYPE_FLOAT,
    UNIFORM_TYPE_VEC2,
    UNIFORM_TYPE_VEC3,
    UNIFORM_TYPE_VEC4,
    UNIFORM_TYPE_INT,
    UNIFORM_TYPE_IVEC2,
    UNIFORM_TYPE_IVEC3,
    UNIFORM_TYPE_IVEC4,
    UNIFORM_TYPE_UINT,
    UNIFORM_TYPE_UVEC2,
    UNIFORM_TYPE_UVEC3,
    UNIFORM_TYPE_UVEC4,
    UNIFORM_TYPE_MAT3X3,
    UNIFORM_TYPE_MAT4X4,
    UNIFORM_TYPE_STRUCT,
    UNIFORM_TYPE_MAX
};

using UniformValue = std::variant<
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,
    unsigned int,
    glm::uvec2,
    glm::uvec3,
    glm::uvec4,
    glm::mat3,
    glm::mat4>;

class Uniform : public RenderResource {
public:
    virtual Uniform& setType(UniformType f_type);
    virtual Uniform& setName(const std::string& f_name);
    virtual Uniform& create();

    template <typename T>
    Uniform& setValue(const T& f_value);

    virtual size_t getSize() const;
    virtual size_t getAlignment() const = 0;

protected:
    UniformType  m_type;
    std::string  m_name;
    UniformValue m_value;
};

}   // namespace uniform
}   // namespace renderer

#endif
