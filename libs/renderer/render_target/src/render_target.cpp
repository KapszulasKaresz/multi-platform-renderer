#include "renderer/render_target/inc/render_target.hpp"

#include <stdexcept>

namespace renderer {
namespace render_target {
bool RenderTarget::isValid() const
{
    return m_valid;
}

RenderTarget& RenderTarget::setFormat(image::ImageFormat f_format)
{
    if (isValid()) {
        throw std::runtime_error(
            "RenderTarget::setFormat(image::ImageFormat f_format) you cannot set format "
            "on an already created target"
        );
    }

    m_format = f_format;
    return *this;
}

RenderTarget& RenderTarget::setColorSpace(image::ColorSpace f_colorSpace)
{
    if (isValid()) {
        throw std::runtime_error(
            "RenderTarget::setColorSpace(image::ColorSpace f_colorSpace) you cannot set "
            "color space on an already created target"
        );
    }

    m_colorSpace = f_colorSpace;
    return *this;
}
}   // namespace render_target
}   // namespace renderer
