#include "renderer/material/inc/material.hpp"

#include <stdexcept>

namespace renderer {
namespace material {
Material& Material::setShader(std::string f_shaderPath)
{
    m_shaderLocation = f_shaderPath;
    return *this;
}

Material& Material::copyMaterial(std::shared_ptr<Material> f_other)
{
    m_original = f_other;

    for (int index = 0; index < f_other->m_uniformCollections.size(); ++index) {
        if (f_other->m_uniformCollections[index]->isUniqueCollection()) {
            m_uniformCollections.push_back(f_other->m_uniformCollections[index]);
        }
        else {
            m_uniformCollections.push_back(
                f_other->m_uniformCollections[index]->deepCopy()
            );
        }
    }

    for (int index = 0; index < f_other->m_uniformArrays.size(); ++index) {
        m_uniformArrays.push_back(f_other->m_uniformArrays[index]);
        // TODO not deep copy if needed
    }

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

Material& Material::addUniformArray(std::shared_ptr<uniform::UniformArray> f_uniformArray)
{
    m_uniformArrays.push_back(f_uniformArray);
    return *this;
}

std::shared_ptr<uniform::UniformArray> Material::getUniformArray(std::string_view f_name)
{
    auto l_it = std::find_if(
        m_uniformArrays.begin(),
        m_uniformArrays.end(),
        [&](const std::shared_ptr<uniform::UniformArray>& f_member) {
            return f_member->getName() == f_name;
        }
    );

    if (l_it != m_uniformArrays.end()) {
        return *l_it;
    }

    throw std::runtime_error("Material::getUniformArray(...) Uniform not found.");
}

bool Material::isOriginal() const
{
    return !m_original;
}

void Material::updateUniforms()
{
    for (const auto& l_uniform : m_uniformCollections) {
        l_uniform->update();
    }
}

}   // namespace material
}   // namespace renderer
