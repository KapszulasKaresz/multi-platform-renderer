#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include <string_view>

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace image {

enum ImageFormat {
    IMAGE_FORMAT_UNDEFINED,
    IMAGE_FORMAT_RGBA8,
    IMAGE_FORMAT_RGBA8_SRGB,
    IMAGE_FORMAT_RGB8,
    IMAGE_FORMAT_RGB8_SRGB,
    IMAGE_FORMAT_BGRA8,
    IMAGE_FORMAT_BGRA8_SRGB,
    IMAGE_FORMAT_DEPTH,
    IMAGE_FORMAT_MAX
};

enum ColorSpace {
    COLOR_SPACE_LINEAR,
    COLOR_SPACE_SRGB_NON_LINEAR,
    COLOR_SPACE_MAX
};

class Image : public RenderResource {
public:
    virtual Image& setFormat(image::ImageFormat f_format);
    virtual Image& setColorSpace(image::ColorSpace f_colorSpace);
    virtual Image& setSize(const glm::ivec2& f_size);
    virtual Image& setWidth(int f_width);
    virtual Image& setHeight(int f_height);
    virtual Image& createFromFile(std::string_view f_path) = 0;
    virtual Image& createEmptyImage()                      = 0;
    virtual Image& generateMipMaps(bool f_generate = true);

    virtual Image& create() = 0;

    uint32_t   getMipLevels() const;
    glm::ivec2 getSize() const;

protected:
    bool isDepthImage() const;

    image::ImageFormat m_format{ image::ImageFormat::IMAGE_FORMAT_UNDEFINED };
    image::ColorSpace  m_colorSpace{ image::ColorSpace::COLOR_SPACE_LINEAR };

    bool m_hasMipMaps{ false };

    uint32_t   m_mipLevels{ 1 };
    glm::ivec2 m_size;
};

}   // namespace image
}   // namespace renderer


#endif
