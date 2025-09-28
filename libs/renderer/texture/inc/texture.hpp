#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

#include <memory>

#include "renderer/image/inc/image.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace texture {
class Texture : public RenderResource {
public:
    virtual Texture& setImage(std::shared_ptr<image::Image> f_image);
    virtual Texture& create() = 0;

private:
    std::shared_ptr<image::Image> m_image{ nullptr };
};
}   // namespace texture
}   // namespace renderer

#endif
