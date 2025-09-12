#include "renderer/image/inc/image.hpp"

#include <stdexcept>

namespace renderer {
namespace image {
bool Image::isValid() const
{
    return m_valid;
}

Image& Image::setFormat(image::ImageFormat f_format)
{
    if (isValid()) {
        throw std::runtime_error(
            "Image::setFormat(image::ImageFormat f_format) you cannot set format "
            "on an already created target"
        );
    }

    m_format = f_format;
    return *this;
}

Image& Image::setColorSpace(image::ColorSpace f_colorSpace)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "RenderTarget::setColorSpace(image::ColorSpace f_colorSpace) you cannot "
                "set " "color space on an already created target"
            );
    }

    m_colorSpace = f_colorSpace;
    return *this;
}

}   // namespace image
}   // namespace renderer
