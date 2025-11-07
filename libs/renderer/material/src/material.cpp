#include "renderer/material/inc/material.hpp"

#include <stdexcept>

namespace renderer {
namespace material {
Material& Material::setShader(std::string f_shaderPath)
{
    m_shaderLocation = f_shaderPath;
    return *this;
}

Material& Material::addUniformCollection(
    std::shared_ptr<uniform::UniformCollection> f_uniformCollection
)
{
    m_uniformCollections.push_back(f_uniformCollection);
    return *this;
}

std::shared_ptr<uniform::UniformCollection> Material::getUniformCollection(
    std::string_view f_name
)
{
    auto l_it = std::find_if(
        m_uniformCollections.begin(),
        m_uniformCollections.end(),
        [&](const std::shared_ptr<uniform::UniformCollection>& f_member) {
            return f_member->getName() == f_name;
        }
    );

    if (l_it != m_uniformCollections.end()) {
        return *l_it;
    }

    throw std::runtime_error("Material::getUniformCollection(...) Uniform not found.");
}

void Material::updateUniforms()
{
    for (const auto& l_uniform : m_uniformCollections) {
        l_uniform->update();
    }
}

}   // namespace material
}   // namespace renderer
