#include "renderer/render_target/inc/render_target.hpp"

#include <stdexcept>

namespace renderer {
namespace render_target {

RenderTarget& RenderTarget::setFormat(image::ImageFormat f_format)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "RenderTarget::setFormat(image::ImageFormat f_format) you cannot set "
                "format " "on an already created target"
            );
    }

    m_format = f_format;
    return *this;
}

RenderTarget& RenderTarget::setColorSpace(image::ColorSpace f_colorSpace)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "RenderTarget::setColorSpace(image::ColorSpace f_colorSpace) you cannot "
                "set color space on an already created target"
            );
    }

    m_colorSpace = f_colorSpace;
    return *this;
}

RenderTarget& RenderTarget::setDepthBuffer(bool f_useDepthBuffer)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "RenderTarget::setDepthBuffer(bool f_useDepthBuffer) you cannot enable "
                "depth " "buffer once render target is created"
            );
    }

    m_useDepthBuffer = f_useDepthBuffer;
    return *this;
}

bool RenderTarget::isDepthBufferEnabled() const
{
    return m_useDepthBuffer;
}

image::ImageFormat RenderTarget::getFormat() const
{
    return m_format;
}

}   // namespace render_target
}   // namespace renderer
