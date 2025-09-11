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

private:
};

}   // namespace image
}   // namespace renderer


#endif
