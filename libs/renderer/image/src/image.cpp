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

Image& Image::generateMipMaps(bool f_generate)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "Image::generateMipMaps(bool f_generate) you cannot set generate mipmaps "
                "on " "an already created image"
            );
    }

    m_hasMipMaps = f_generate;
    return *this;
}

uint32_t Image::getMipLevels() const
{
    return m_mipLevels;
}

glm::ivec2 Image::getSize() const
{
    return m_size;
}

image::ImageFormat Image::getFormat() const
{
    return m_format;
}

bool Image::isDepthImage() const
{
    return m_format == IMAGE_FORMAT_DEPTH;
}

std::vector<unsigned char> Image::expandToRGBA(const tinygltf::Image& f_gltfImage)
{
    size_t                     l_pixelCount = f_gltfImage.width * f_gltfImage.height;
    std::vector<unsigned char> l_rgbaData(l_pixelCount * 4);

    for (size_t i = 0; i < l_pixelCount; ++i) {
        l_rgbaData[i * 4 + 0] = f_gltfImage.image[i * 3 + 0];   // R
        l_rgbaData[i * 4 + 1] = f_gltfImage.image[i * 3 + 1];   // G
        l_rgbaData[i * 4 + 2] = f_gltfImage.image[i * 3 + 2];   // B
        l_rgbaData[i * 4 + 3] = 255;                            // A (Opaque)
    }
    return l_rgbaData;
}

}   // namespace image
}   // namespace renderer
