#include "renderer/texture/inc/texture.hpp"

namespace renderer {
namespace texture {
Texture& Texture::setImage(std::shared_ptr<image::Image> f_image)
{
    m_image = f_image;
    return *this;
}

std::string Texture::getName() const
{
    return m_name;
}

Texture& Texture::setName(const std::string& f_name)
{
    m_name = f_name;
    return *this;
}
}   // namespace texture
}   // namespace renderer
