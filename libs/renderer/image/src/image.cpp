#include "renderer/image/inc/image.hpp"

#include <stdexcept>

namespace renderer {
namespace image {

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
                "Image::setColorSpace(image::ColorSpace f_colorSpace) you cannot "
                "set color space on an already created target"
            );
    }

    m_colorSpace = f_colorSpace;
    return *this;
}

Image& Image::setSize(const glm::ivec2& f_size)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "Image::setSize(const glm::ivec2& f_size) you cannot set size on an "
                "already created image"
            );
    }

    m_size = f_size;
    return *this;
}

Image& Image::setWidth(int f_width)
{
    if (isValid()) {
        throw std::runtime_error(
            "Image::setWidth(int f_width) you cannot set width on an already created "
            "image"
        );
    }
    m_size.x = f_width;
    return *this;
}

Image& Image::setHeight(int f_height)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "Image::setHeight(int f_height) you cannot set height on an already "
                "created " "image"
            );
    }

    m_size.y = f_height;
    return *this;
}

bool Image::isDepthImage() const
{
    return m_format == IMAGE_FORMAT_DEPTH;
}

}   // namespace image
}   // namespace renderer
