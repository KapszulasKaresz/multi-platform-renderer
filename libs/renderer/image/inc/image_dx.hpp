#ifndef IMAGE_DX_HPP_INCLUDED
#define IMAGE_DX_HPP_INCLUDED

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/image/inc/image.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace image {

class ImageDX : public Image {
public:
    ImageDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    ImageDX& create() override final;
    ImageDX& createFromFile(std::string_view f_path) override final;
    ImageDX& createEmptyImage() override final;

    ImageDX& setFormat(image::ImageFormat f_format) override final;
    ImageDX& setColorSpace(image::ColorSpace f_colorSpace) override final;
    ImageDX& setSize(const glm::ivec2& f_size) override final;

    static DXGI_FORMAT convertToDXFormat(const ImageFormat f_format);

private:
    rendering_device::RenderingDeviceDX* m_parentDevice;
};
}   // namespace image
}   // namespace renderer

#endif
