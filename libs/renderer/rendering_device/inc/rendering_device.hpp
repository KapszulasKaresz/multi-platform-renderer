#ifndef RENDERING_DEVICE_HPP_INCLUDED
#define RENDERING_DEVICE_HPP_INCLUDED

#include <memory>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace material {
class Material;
}   // namespace material

namespace image {
class Image;
}   // namespace image

namespace render_target {
class RenderTargetWindow;
}   // namespace render_target

namespace window {
class Window;
}   // namespace window

namespace rendering_device {
class RenderingDevice : public RenderResource {
public:
    virtual render_target::RenderTargetWindow* getRenderTargetWindow() = 0;

    virtual RenderingDevice& setWindow(window::Window* f_window) = 0;

    virtual std::shared_ptr<image::Image>       createImage()    = 0;
    virtual std::shared_ptr<material::Material> createMaterial() = 0;

    virtual bool isValid();

protected:
};

}   // namespace rendering_device
}   // namespace renderer

#endif
