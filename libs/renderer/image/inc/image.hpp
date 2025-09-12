#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

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
    IMAGE_FORMAT_MAX
};

enum ColorSpace {
    COLOR_SPACE_LINEAR,
    COLOR_SPACE_SRGB_NON_LINEAR,
    COLOR_SPACE_MAX
};

class Image {
public:
    virtual bool isValid() const;

    virtual Image& setFormat(image::ImageFormat f_format);
    virtual Image& setColorSpace(image::ColorSpace f_colorSpace);

    virtual Image& create() = 0;

protected:
    bool m_valid{ false };

    image::ImageFormat m_format{ image::ImageFormat::IMAGE_FORMAT_UNDEFINED };
    image::ColorSpace  m_colorSpace{ image::ColorSpace::COLOR_SPACE_LINEAR };
};

}   // namespace image
}   // namespace renderer


#endif
