#ifndef RENDER_TARGET_HPP_INCLUDED
#define RENDER_TARGET_HPP_INCLUDED
#include "renderer/image/inc/image.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace render_target {

class RenderTarget : public RenderResource {
public:
    RenderTarget&         setFormat(image::ImageFormat f_format);
    RenderTarget&         setColorSpace(image::ColorSpace f_colorSpace);
    virtual RenderTarget& create() = 0;

    virtual image::ImageFormat getFormat() const;

protected:
    image::ImageFormat m_format{ image::ImageFormat::IMAGE_FORMAT_UNDEFINED };
    image::ColorSpace  m_colorSpace{ image::ColorSpace::COLOR_SPACE_LINEAR };
};

}   // namespace render_target
}   // namespace renderer

#endif
