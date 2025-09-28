#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

#include <memory>
#include <string>

#include "renderer/image/inc/image.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace texture {
class Texture : public RenderResource {
public:
    virtual Texture& setImage(std::shared_ptr<image::Image> f_image);
    virtual Texture& create() = 0;

    std::string getName() const;
    Texture&    setName(const std::string& f_name);

protected:
    std::shared_ptr<image::Image> m_image{ nullptr };
    std::string                   m_name{ "" };
};
}   // namespace texture
}   // namespace renderer

#endif
