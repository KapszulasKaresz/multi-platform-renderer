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

Texture& Texture::setMinFilter(Filter f_filter)
{
    m_minFilter = f_filter;
    return *this;
}

Texture& Texture::setMagFilter(Filter f_filter)
{
    m_magFilter = f_filter;
    return *this;
}

Texture& Texture::setMipMapMode(MipMapMode f_mode)
{
    m_mipMapMode = f_mode;
    return *this;
}

Texture& Texture::setAddressModeU(AddressMode f_addressMode)
{
    m_addressModeU = f_addressMode;
    return *this;
}

Texture& Texture::setAddressModeV(AddressMode f_addressMode)
{
    m_addressModeV = f_addressMode;
    return *this;
}

Texture& Texture::setAddressModeW(AddressMode f_addressMode)
{
    m_addressModeW = f_addressMode;
    return *this;
}


}   // namespace texture
}   // namespace renderer
