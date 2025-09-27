#ifndef UNIFORM_SINGLE_HPP_INCLUDED
#define UNIFORM_SINGLE_HPP_INCLUDED

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {

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

class UniformSingle : public Uniform {
public:
    UniformSingle& setType(UniformType f_type) override;
    UniformSingle& setName(const std::string& f_name) override;

    UniformSingle& setValue(const UniformValue& f_value);

    UniformValue& getValue();
    void*         valueAsVoid();

protected:
    UniformValue m_value;
};

}   // namespace uniform
}   // namespace renderer

#endif
