#ifndef RENDER_TARGET_HPP_INCLUDED
#define RENDER_TARGET_HPP_INCLUDED
#include <glm/glm.hpp>

#include "renderer/image/inc/image.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace render_target {

class RenderTarget : public RenderResource {
public:
    RenderTarget&         setFormat(image::ImageFormat f_format);
    RenderTarget&         setColorSpace(image::ColorSpace f_colorSpace);
    RenderTarget&         setDepthBuffer(bool f_useDepthBuffer);
    virtual RenderTarget& create() = 0;

    bool isDepthBufferEnabled() const;

    virtual image::ImageFormat getFormat() const;

    virtual std::shared_ptr<image::Image> getImage()      = 0;
    virtual std::shared_ptr<image::Image> getDepthImage() = 0;
    virtual glm::ivec2                    getSize() const = 0;

protected:
    image::ImageFormat m_format{ image::ImageFormat::IMAGE_FORMAT_UNDEFINED };
    image::ColorSpace  m_colorSpace{ image::ColorSpace::COLOR_SPACE_LINEAR };

    bool m_useDepthBuffer{ false };
};

}   // namespace render_target
}   // namespace renderer

#endif
