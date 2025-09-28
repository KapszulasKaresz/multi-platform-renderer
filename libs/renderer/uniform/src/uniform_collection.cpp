#include "renderer/uniform/inc/uniform_collection.hpp"

#include <algorithm>
#include <stdexcept>

namespace renderer {
namespace uniform {
Uniform& UniformCollection::setType(UniformType f_type)
{
    return *this;
}

void UniformCollection::addMember(std::unique_ptr<Uniform> f_member, int f_position)
{
    if (f_position < 0) {
        m_members.push_back(std::move(f_member));
    }
    else {
        if (m_members.size() <= f_position) {
            m_members.resize(f_position - 1);
        }
        m_members[f_position] = std::move(f_member);
    }
}

Uniform* UniformCollection::getMember(std::string_view f_name)
{
    auto l_it = std::find_if(
        m_members.begin(), m_members.end(), [&](const std::unique_ptr<Uniform>& f_member) {
            return f_member->getName() == f_name;
        }
    );

    if (l_it != m_members.end()) {
        return l_it->get();
    }

    throw std::runtime_error(" UniformCollection::getMember(...) Uniform not found.");
}

void UniformCollection::addTexture(
    std::shared_ptr<texture::Texture> f_textrue,
    int                               f_position
)
{
    if (f_position < 0) {
        m_textures.push_back(std::move(f_textrue));
    }
    else {
        if (m_textures.size() <= f_position) {
            m_textures.resize(f_position - 1);
        }
        m_textures[f_position] = std::move(f_textrue);
    }
}

texture::Texture* UniformCollection::getTexture(std::string_view f_name)
{
    auto l_it = std::find_if(
        m_textures.begin(),
        m_textures.end(),
        [&](const std::shared_ptr<texture::Texture>& f_texture) {
            return f_texture->getName() == f_name;
        }
    );

    if (l_it != m_textures.end()) {
        return l_it->get();
    }

    throw std::runtime_error("UniformCollection::getTexture(...) texture not found.");
}


}   // namespace uniform
}   // namespace renderer
