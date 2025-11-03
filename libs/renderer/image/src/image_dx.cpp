#include "renderer/image/inc/image_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace image {
ImageDX::ImageDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : Image(),
      m_parentDevice(f_parentDevice)
{}

ImageDX& ImageDX::create()
{
    throw std::logic_error("Function not yet implemented");
}

ImageDX& ImageDX::createFromFile(std::string_view f_path)
{
    throw std::logic_error("Function not yet implemented");
}

ImageDX& ImageDX::createEmptyImage()
{
    throw std::logic_error("Function not yet implemented");
}

ImageDX& ImageDX::setFormat(image::ImageFormat f_format)
{
    m_format = f_format;
    return *this;
}

ImageDX& ImageDX::setColorSpace(image::ColorSpace f_colorSpace)
{
    m_colorSpace = f_colorSpace;
    return *this;
}

ImageDX& ImageDX::setSize(const glm::ivec2& f_size)
{
    m_size = f_size;
    return *this;
}

DXGI_FORMAT ImageDX::convertToDXFormat(const ImageFormat f_format)
{
    switch (f_format) {
        case IMAGE_FORMAT_RGBA8: {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }

        case IMAGE_FORMAT_RGBA8_SRGB: {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }

        // Note: DirectX lacks a native 24-bit format.
        // We map to a 32-bit format and the alpha channel will be ignored.
        case IMAGE_FORMAT_RGB8: {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        }

        case IMAGE_FORMAT_RGB8_SRGB: {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }

        case IMAGE_FORMAT_BGRA8: {
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        }

        case IMAGE_FORMAT_BGRA8_SRGB: {
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }

        case IMAGE_FORMAT_UNDEFINED:
        case IMAGE_FORMAT_MAX:
        default:                     {
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

}   // namespace image
}   // namespace renderer
