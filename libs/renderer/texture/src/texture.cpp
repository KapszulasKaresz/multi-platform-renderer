#include "renderer/texture/inc/texture.hpp"

namespace renderer {
namespace texture {
Texture& Texture::setImage(std::shared_ptr<image::Image> f_image)
{
    m_image = f_image;
    return *this;
}
}   // namespace texture
}   // namespace renderer
